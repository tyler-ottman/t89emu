#include "DwarfParser.h"

DataStream::DataStream(uint8_t *data) : data(data) {}

DataStream::~DataStream() {}

uint8_t DataStream::decodeUInt8() {
    uint16_t res = data[0];
    data++;
    return res;
}

uint16_t DataStream::decodeUInt16() {
    uint16_t res = data[0] | data[1] << 8;
    data += 2;
    return res;
}

uint32_t DataStream::decodeUInt32() {
    uint16_t res = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;
    data += 4;
    return res;
}

int64_t DataStream::decodeLeb128() {
    size_t result = 0;
    size_t shift = 0;
    uint8_t byte;
    
    for (;;) {
        byte = *(data++);
        result |= (0x7f & byte) << shift;
        shift += 7;
        if ((0x80 & byte) == 0) { break; }
    }
    
    if (0x40 & byte) {
        result |= -(1 << shift);
    }

    return result;
}

size_t DataStream::decodeULeb128() {
    size_t result = 0;
    size_t shift = 0;
    
    for (;;) {
        result |= (0x7f & *data) << shift;
        if (!(0x80 & *(data++))) { break; }
        shift += 7;
    }

    return result;
}

const uint8_t *DataStream::getData() { return data; }

DebugData::DebugData(FormEncoding form) : form(form) {}

DebugData::~DebugData() {}

void DebugData::write(uint8_t *buff, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data.push_back(buff[i]);
    }
}

bool DebugData::isString() {
    return form == DW_FORM_string || form == DW_FORM_strp ||
           form == DW_FORM_line_strp;
}

FormEncoding DebugData::getForm() { return form; }

uint64_t DebugData::getUInt() {
    uint64_t res = 0;

    switch (form) {
    case DW_FORM_addr:
    case DW_FORM_block:
    case DW_FORM_exprloc:
    case DW_FORM_data1:
    case DW_FORM_flag:
    case DW_FORM_flag_present:
    case DW_FORM_ref1:
    case DW_FORM_strx1:
    case DW_FORM_addrx1:
    case DW_FORM_data2:
    case DW_FORM_ref2:
    case DW_FORM_strx2:
    case DW_FORM_addrx2:
    case DW_FORM_strx3:
    case DW_FORM_addrx3:
    case DW_FORM_data4:
    case DW_FORM_ref_addr:
    case DW_FORM_ref4:
    case DW_FORM_sec_offset:
    case DW_FORM_line_strp:
    case DW_FORM_strx4:
    case DW_FORM_addrx4:
    case DW_FORM_data8:
    case DW_FORM_ref8:
    case DW_FORM_ref_sig8:
    case DW_FORM_sdata:
    case DW_FORM_udata:
    case DW_FORM_ref_udata: 
    case DW_FORM_strx:
    case DW_FORM_addrx:
    case DW_FORM_loclistx:
    case DW_FORM_rnglistx:
    case DW_FORM_implicit_const: {
        for (size_t i = 0; i < data.size(); i++) {
            res |= (uint64_t)data[i] << (8*i);
        }
        break;
    }

    default: // FORM does not support reading as int
        std::cerr << "Unsupported form: " << form << std::endl;
        exit(1);
    }
    return res;
}

const char *DebugData::getString() {
    return (new std::string(data.begin(), data.end()))->c_str();
}

AttributeEntry::AttributeEntry(AttributeEncoding name, FormEncoding form,
                               size_t special)
    : name(name), form(form), special(special) {}

AttributeEntry::~AttributeEntry() {}

AttributeEncoding AttributeEntry::getName() { return name; }

FormEncoding AttributeEntry::getForm() { return form; }

size_t AttributeEntry::getSpecial() { return special; }

AbbrevEntry::AbbrevEntry(size_t dieCode, TagEncoding dieTag, bool hasChild)
    : dieCode(dieCode), dieTag(dieTag), hasChild(hasChild) {}

AbbrevEntry::~AbbrevEntry() {
    for (AttributeEntry *attEntry : attEntries) {
        delete attEntry;
    }

    attEntries.clear();
}

void AbbrevEntry::addAttributeEntry(AttributeEntry *attEntry) {
    attEntries.push_back(attEntry);
}

size_t AbbrevEntry::getNumAttributes(void) { return attEntries.size(); }

AttributeEntry *AbbrevEntry::getAttributeEntry(size_t index) {
    if (index < 0 || index >= attEntries.size()) {
        return nullptr;
    }
    return attEntries[index];
}

size_t AbbrevEntry::getDieCode() { return dieCode; }

TagEncoding AbbrevEntry::getDieTag() { return dieTag; }

bool AbbrevEntry::hasChildren() { return hasChild; }

AbbrevTable::AbbrevTable(uint8_t *abbrevTableStart) {
    abbrevData = new DataStream(abbrevTableStart);

    for (;;) {
        size_t dieCode = abbrevData->decodeULeb128();
        if (dieCode == 0) { // End of Compile Unit
            break;
        }

        TagEncoding dieTag = (TagEncoding)abbrevData->decodeULeb128();
        size_t hasChild = abbrevData->decodeULeb128();
        AbbrevEntry *abbrevEntry = new AbbrevEntry(dieCode, dieTag, hasChild);

        for (;;) {
            AttributeEncoding name =
                (AttributeEncoding)abbrevData->decodeULeb128();
            FormEncoding form = (FormEncoding)abbrevData->decodeULeb128();
            if (name == 0 && form == 0) { break; }
            size_t special = (form == DW_FORM_implicit_const)
                             ? abbrevData->decodeULeb128() : 0;

            abbrevEntry->addAttributeEntry(
                new AttributeEntry(name, form, special));
        }

        abbrevEntries.insert({dieCode, abbrevEntry});
    }
}

AbbrevTable::~AbbrevTable() {
    for (auto const &abbrevEntry : abbrevEntries) {
        delete abbrevEntry.second;
    }
    abbrevEntries.clear();

    delete abbrevData;
}

AbbrevEntry *AbbrevTable::getAbbrevEntry(size_t dieCode) {
    return abbrevEntries.find(dieCode) != abbrevEntries.end()
               ? abbrevEntries[dieCode]
               : nullptr;
}

CompileUnitHeader::CompileUnitHeader(uint8_t *debugInfoCompileUnitHeader) {
    memcpy(&header, debugInfoCompileUnitHeader, sizeof(FullCompileUnitHeader));
    ASSERT(getAddressSize() == 4, "addressSize error");
}

CompileUnitHeader::~CompileUnitHeader() {}

uint32_t CompileUnitHeader::getUnitLength() { return header.base.unitLength; }

uint16_t CompileUnitHeader::getUnitVersion() { return header.base.version; }

uint8_t CompileUnitHeader::getUnitType() { return header.base.unitType; }

uint8_t CompileUnitHeader::getAddressSize() { return header.base.addressSize; }

uint32_t CompileUnitHeader::getDebugAbbrevOffset() {
    return header.base.debugAbbrevOffset;
}

DebugInfoEntry::DebugInfoEntry(CompileUnit *compileUnit, DebugInfoEntry *parent)
    : compileUnit(compileUnit), parent(parent), code(0) {}

DebugInfoEntry::~DebugInfoEntry() {
    for (DebugInfoEntry *debugEntry : children) {
        delete debugEntry;
    }
}

void DebugInfoEntry::addAttribute(AttributeEncoding encoding, DebugData *data) {
    attributes.insert({encoding, data});
}

void DebugInfoEntry::addChild(DebugInfoEntry *child) {
    children.push_back(child);
}

void DebugInfoEntry::printEntry() {
    printf("\nCode: 0x%02lx %s\n", code, printTag(abbrevEntry->getDieTag()));
    for (size_t i = 0; i < abbrevEntry->getNumAttributes(); i++) {
        // Use abbrevEntry so attributes printed in correct order
        AttributeEntry *attEntry = abbrevEntry->getAttributeEntry(i);
        printf("\t%-30s%-23s", printAttribute(attEntry->getName()),
               printForm(attEntry->getForm()));
        DebugData *data = attributes[attEntry->getName()];
        if (data->isString()) { printf("(%s)\n", data->getString()); }
        else { printf("(%lx)\n", data->getUInt()); }
    }
}

AbbrevEntry *DebugInfoEntry::getAbbrevEntry() { return abbrevEntry; }

size_t DebugInfoEntry::getCode() { return code; }

DebugInfoEntry *DebugInfoEntry::getParent() { return parent; }

void DebugInfoEntry::setAbbrevEntry(AbbrevEntry *abbrevEntry) {
    this->abbrevEntry = abbrevEntry;
}

void DebugInfoEntry::setCode(size_t dieCode) { code = dieCode; }

CompileUnit::CompileUnit(uint8_t *debugInfoCUHeader, uint8_t *debugAbbrevStart,
                         uint8_t *debugStrStart, uint8_t *debugLineStrStart) {
    compileUnitHeader = new CompileUnitHeader(debugInfoCUHeader);
    abbrevTable = new AbbrevTable(debugAbbrevStart +
                                  compileUnitHeader->getDebugAbbrevOffset());
    size_t headerLen = sizeof(CompileUnitHeader::FullCompileUnitHeader);
    debugStream = new DataStream(debugInfoCUHeader + headerLen);
    root = new DebugInfoEntry(this, nullptr);
    debugStr = new StringTable((char *)debugStrStart);
    debugLineStr = new StringTable((char *)debugLineStrStart);
    debugStart = (uintptr_t)debugInfoCUHeader;

    generateDebugInfo(root);
}

CompileUnit::~CompileUnit() {
    delete compileUnitHeader;
    delete abbrevTable;
}

AbbrevEntry *CompileUnit::getAbbrevEntry(size_t dieCode) {
    return abbrevTable->getAbbrevEntry(dieCode);
}

size_t CompileUnit::getAddrSize() {
    return compileUnitHeader->getAddressSize();
}

size_t CompileUnit::getLength() {
    // size of length-field (4 bytes) + Rest of CU Header + DIEs
    return 4 + compileUnitHeader->getUnitLength();
}

DebugInfoEntry *CompileUnit::generateDebugInfo(DebugInfoEntry *node) {
    size_t code = debugStream->decodeULeb128();
    if (code == 0) { // End of siblings, travel back up tree
        delete node;
        return nullptr;
    }

    node->setCode(code);
    node->setAbbrevEntry(getAbbrevEntry(node->getCode()));
    
    AbbrevEntry *dieAbbrevEntry = node->getAbbrevEntry();
    ASSERT(dieAbbrevEntry, "dieAbbrevEntry not found\n");
    for (size_t i = 0; i < dieAbbrevEntry->getNumAttributes(); i++) {
        AttributeEntry *attEntry = dieAbbrevEntry->getAttributeEntry(i);
        DebugData *data = decodeInfo(attEntry);
        node->addAttribute(attEntry->getName(), data);
    }

    node->printEntry();

    // Check if end of CU reached or if leaf node
    if (((uintptr_t)debugStream->getData() == debugStart + getLength())) {
        return nullptr;
    } else if (!dieAbbrevEntry->hasChildren()) {
        return node;
    }

    // Traverse child subtrees
    DebugInfoEntry *child;
    while ((child = generateDebugInfo(new DebugInfoEntry(this, node)))) {
        node->addChild(child);
    }

    return node;    
}

DebugData *CompileUnit::decodeInfo(AttributeEntry *entry) {
    FormEncoding form = entry->getForm();
    DebugData *dieData = new DebugData(form);
    size_t streamLen = 0;

    switch (form) {    
    case DW_FORM_addr: // Read addr_size bytes from stream
        streamLen = compileUnitHeader->getAddressSize();
        break;
    
    case DW_FORM_block: // Read ULEB128 length, then block
    case DW_FORM_exprloc:
        streamLen = debugStream->decodeULeb128();
        break;
    case DW_FORM_block1: // Read 1-byte length, then block
        streamLen = debugStream->decodeUInt8();
        break;
    case DW_FORM_block2: // Read 2-byte length, then block
        streamLen = debugStream->decodeUInt16();
        break;
    case DW_FORM_block4: // Read 4-byte length, then block
        streamLen = debugStream->decodeUInt32();
        break;

    case DW_FORM_flag_present: break; // Read 0-byte integer

    case DW_FORM_data1: // Read 1-byte integer
    case DW_FORM_flag:
    case DW_FORM_ref1:
    case DW_FORM_strx1:
    case DW_FORM_addrx1: streamLen = 1; break;

    case DW_FORM_data2: // Read 2-byte integer
    case DW_FORM_ref2:
    case DW_FORM_strx2:
    case DW_FORM_addrx2: streamLen = 2; break;
    
    case DW_FORM_strx3:
    case DW_FORM_addrx3: streamLen = 3; break; // Read 3-byte integer

    case DW_FORM_data4: // Read 4-byte integer
    case DW_FORM_ref_addr:
    case DW_FORM_ref4:
    case DW_FORM_sec_offset:
    case DW_FORM_strx4:
    case DW_FORM_addrx4: streamLen = 4; break;
    
    case DW_FORM_data8: // Read 8-byte integer
    case DW_FORM_ref8:
    case DW_FORM_ref_sig8: streamLen = 8; break;
    
    case DW_FORM_data16: streamLen = 16; break; // Read 16-byte integer

    case DW_FORM_sdata: { // Read LEB128
        int64_t value = debugStream->decodeLeb128();
        dieData->write((uint8_t *)&value, sizeof(int64_t));
        break;
    }

    case DW_FORM_udata: // Read ULEB128
    case DW_FORM_ref_udata: 
    case DW_FORM_strx:
    case DW_FORM_addrx:
    case DW_FORM_loclistx:
    case DW_FORM_rnglistx: {
        uint64_t value = debugStream->decodeULeb128();
        dieData->write((uint8_t *)&value, sizeof(uint64_t));
        break;
    }

     // Read offset, copy string from either .debug_str or .debug_line_str
    case DW_FORM_strp: {
    case DW_FORM_line_strp:
        int offset = debugStream->decodeUInt32();

        StringTable *strTable = form == DW_FORM_strp ? debugStr : debugLineStr;
        std::string str = strTable->getString(offset);
        dieData->write((uint8_t *)str.c_str(), str.size());
        break;
    }

    case DW_FORM_string: { // Read string directly from DIE
        uint8_t c;
        while ((c = debugStream->decodeUInt8()) != '\0') {
            dieData->write(&c, sizeof(uint8_t));
        }
        break;
    }

    case DW_FORM_implicit_const: {// special value is attribute value
        size_t value = entry->getSpecial();
        dieData->write((uint8_t *)&value, sizeof(size_t));
        break;
    }

    default: // indirect, ref_sup4, strp_sup, ref_sup8
        std::cerr << "Unsupported form: " << form << std::endl;
        exit(0);
        break;
    }

    // If streamLen is used by a FORM, directly write bytes from debugStream
    for (; streamLen > 0; streamLen--) {
        uint8_t byte = debugStream->decodeUInt8();
        dieData->write(&byte, sizeof(byte));
    }

    return dieData;
}

StringTable::StringTable(char *tableStart) : tableStart(tableStart) {}

StringTable::~StringTable() {}

std::string StringTable::getString(size_t offset) {
    std::string res;

    for (int i = offset; tableStart[i] != '\0'; i++) {
        res += tableStart[i];
    }

    return res;
}

DwarfParser::DwarfParser(const char *fileName)
    : ElfParser::ElfParser(fileName) {
    const ElfSectionHeader *debugInfoHeader = getSectionHeader(".debug_info");
    const ElfSectionHeader *debugAbbrevHeader =
        getSectionHeader(".debug_abbrev");
    const ElfSectionHeader *debugStrHeader = getSectionHeader(".debug_str");
    const ElfSectionHeader *debugLineStrHeader =
        getSectionHeader(".debug_line_str");

    uint8_t *fileStart = elfFileInfo->elfData;

    uint8_t *debugInfoCUHeader = fileStart + debugInfoHeader->offset;
    uint8_t *debugInfoEnd = debugInfoCUHeader + debugInfoHeader->size;
    uint8_t *debugAbbrevStart = fileStart + debugAbbrevHeader->offset;
    uint8_t *debugStrStart = fileStart + debugStrHeader->offset;
    uint8_t *debugLineStrStart = fileStart + debugLineStrHeader->offset;

    // Initialize Compile Units
    for (;;) {
        // End of .debug_info section, CU parsing complete
        if ((uintptr_t)debugInfoCUHeader >= ((uintptr_t)debugInfoEnd)) {
            break;
        }

        CompileUnit *compileUnit = new CompileUnit(debugInfoCUHeader,
            debugAbbrevStart, debugStrStart, debugLineStrStart);
        compileUnits.push_back(compileUnit);

        // Point to next CU Header
        debugInfoCUHeader += compileUnit->getLength();
    }
}

DwarfParser::~DwarfParser() {
    for (CompileUnit *compileUnit : compileUnits) {
        delete compileUnit;
    }
    compileUnits.clear();
}
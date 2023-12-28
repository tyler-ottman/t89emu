#include "DwarfParser.h"

DebugData::DebugData(FormEncoding form) : form(form) {}

DebugData::~DebugData() {}

uint8_t DebugData::decodeUInt8(uint8_t **data) {
    const uint8_t *bytes = *data;
    uint16_t res = bytes[0];
    (*data)++;
    return res;
}

uint16_t DebugData::decodeUInt16(uint8_t **data) {
    const uint8_t *bytes = *data;
    uint16_t res = bytes[0] | bytes[1] << 8;
    *data += 2;
    return res;
}

uint32_t DebugData::decodeUInt32(uint8_t **data) {
    const uint8_t *bytes = *data;
    uint16_t res = bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;
    *data += 4;
    return res;
}

int64_t DebugData::decodeLeb128(uint8_t **data) {
    size_t result = 0;
    size_t shift = 0;
    uint8_t byte;
    
    for (;;) {
        byte = *(*data)++;
        result |= (0x7f & byte) << shift;
        shift += 7;
        if ((0x80 & byte) == 0) { break; }
    }
    
    if (0x40 & byte) {
        result |= -(1 << shift);
    }

    return result;
}

size_t DebugData::decodeULeb128(uint8_t **data) {
    size_t result = 0;
    size_t shift = 0;
    
    for (;;) {
        result |= (0x7f & **data) << shift;
        if (!(0x80 & *(*data)++)) { break; }
        shift += 7;
    }

    return result;
}

void DebugData::write(uint8_t *buff, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data.push_back(buff[i]);
    }
}

AttributeEntry::AttributeEntry(AttributeEncoding name, FormEncoding form,
                               size_t special)
    : name(name), form(form), special(special) {}

AttributeEntry::~AttributeEntry() {}

AttributeEncoding AttributeEntry::getName() {
    return name;
}

FormEncoding AttributeEntry::getForm() {
    return form;
}

size_t AttributeEntry::getSpecial() {
    return special;
}

AbbrevEntry::AbbrevEntry(size_t dieCode, size_t dieTag, bool hasChild)
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

size_t AbbrevEntry::getNumAttributes(void) {
    return attEntries.size();
}

AttributeEntry *AbbrevEntry::getAttributeEntry(size_t index) {
    if (index < 0 || index >= attEntries.size()) {
        return nullptr;
    }
    return attEntries[index];
}

size_t AbbrevEntry::getDieCode() {
    return dieCode;
}

size_t AbbrevEntry::getDieTag() {
    return dieTag;
}

bool AbbrevEntry::hasChildren() {
    return hasChild;
}

AbbrevTable::AbbrevTable(uint8_t *abbrevTableStart) {
    uint8_t *abbrevData = abbrevTableStart;

    for (;;) {
        size_t dieCode = DebugData::decodeULeb128(&abbrevData);
        if (dieCode == 0) { // End of Compile Unit
            break;
        }

        size_t dieTag = DebugData::decodeULeb128(&abbrevData);
        size_t hasChild = DebugData::decodeULeb128(&abbrevData);
        AbbrevEntry *abbrevEntry = new AbbrevEntry(dieCode, dieTag, hasChild);

        for (;;) {
            AttributeEncoding name =
                (AttributeEncoding)DebugData::decodeULeb128(&abbrevData);
            FormEncoding form =
                (FormEncoding)DebugData::decodeULeb128(&abbrevData);
            if (name == 0 && form == 0) { break; }
            size_t special = (form == DW_FORM_implicit_const)
                             ? DebugData::decodeULeb128(&abbrevData) : 0;

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

uint32_t CompileUnitHeader::getUnitLength() {
    return header.base.unitLength;
}

uint16_t CompileUnitHeader::getUnitVersion() {
    return header.base.version;
}

uint8_t CompileUnitHeader::getUnitType() {
    return header.base.unitType;
}

uint8_t CompileUnitHeader::getAddressSize() {
    return header.base.addressSize;
}

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

AbbrevEntry *DebugInfoEntry::getAbbrevEntry() {
    return abbrevEntry;
}

size_t DebugInfoEntry::getCode() {
    return code;
}

DebugInfoEntry *DebugInfoEntry::getParent() {
    return parent;
}

void DebugInfoEntry::setAbbrevEntry(AbbrevEntry *abbrevEntry) {
    this->abbrevEntry = abbrevEntry;
}

void DebugInfoEntry::setCode(size_t dieCode) {
    code = dieCode;
}

CompileUnit::CompileUnit(uint8_t *debugInfoCUHeader, uint8_t *debugAbbrevStart,
                         uint8_t *debugStrStart) {
    compileUnitHeader = new CompileUnitHeader(debugInfoCUHeader);
    abbrevTable = new AbbrevTable(debugAbbrevStart +
                                  compileUnitHeader->getDebugAbbrevOffset());
    byteStream =
        debugInfoCUHeader + sizeof(CompileUnitHeader::FullCompileUnitHeader);
    root = new DebugInfoEntry(this, nullptr);
    stringTable = new StringTable((char *)debugStrStart);
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

void CompileUnit::generateDebugInfo(DebugInfoEntry *node) {
    size_t code = DebugData::decodeULeb128(&byteStream);
    if (code == 0) { // End of siblings, travel back up tree
        delete node;
        return;
    }

    node->setCode(code);
    node->setAbbrevEntry(getAbbrevEntry(node->getCode()));
    
    AbbrevEntry *dieAbbrevEntry = node->getAbbrevEntry();
    ASSERT(dieAbbrevEntry, "dieAbbrevEntry not found\n");
    for (size_t i = 0; i < dieAbbrevEntry->getNumAttributes(); i++) {
        AttributeEntry *attEntry = dieAbbrevEntry->getAttributeEntry(i);
        // DebugData *data = decodeInfo(attEntry);
        // node->addAttribute(attEntry->getName(), data);
    }

    // Next DIE node is child, sibling, or null entry
    // DebugInfoEntry *next = new DebugInfoEntry(
    //     this, dieAbbrevEntry->hasChildren() ? node : node->getParent());
    
    // generateDebugInfo(next);
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
        streamLen = DebugData::decodeULeb128(&byteStream);
        break;
    case DW_FORM_block1: // Read 1-byte length, then block
        streamLen = DebugData::decodeUInt8(&byteStream);
        break;
    case DW_FORM_block2: // Read 2-byte length, then block
        streamLen = DebugData::decodeUInt16(&byteStream);
        break;
    case DW_FORM_block4: // Read 4-byte length, then block
        streamLen = DebugData::decodeUInt32(&byteStream);
        break;

    case DW_FORM_flag_present: // Read 0-byte integer
        break;

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
    case DW_FORM_line_strp:
    case DW_FORM_strx4:
    case DW_FORM_addrx4: streamLen = 4; break;
    
    case DW_FORM_data8: // Read 8-byte integer
    case DW_FORM_ref8:
    case DW_FORM_ref_sig8: streamLen = 8; break;
    
    case DW_FORM_data16: streamLen = 16; break; // Read 16-byte integer
        

    // Read LEB128
    case DW_FORM_sdata: {
        int64_t value = DebugData::decodeLeb128(&byteStream);
        dieData->write((uint8_t *)&value, sizeof(int64_t));
        break;
    }

    // Read ULEB128
    case DW_FORM_udata: 
    case DW_FORM_ref_udata: 
    case DW_FORM_strx:
    case DW_FORM_addrx:
    case DW_FORM_loclistx:
    case DW_FORM_rnglistx: {
        uint64_t value = DebugData::decodeULeb128(&byteStream);
        dieData->write((uint8_t *)&value, sizeof(uint64_t));
        break;
    }

    case DW_FORM_strp: { // Read offset, then copy string from .debug_str
        int offset = DebugData::decodeUInt32(&byteStream);
        std::string str = stringTable->getString(offset);
        dieData->write((uint8_t *)str.c_str(), str.size());
        break;
    }

    case DW_FORM_string: // Read string directly from DIE
        for (; byteStream[streamLen] != '\0'; streamLen++) {}
        break;

    case DW_FORM_implicit_const: {// special value is attribute value
        size_t value = entry->getSpecial();
        dieData->write((uint8_t *)&value, sizeof(size_t));
        break;
    }

    default:
        std::cerr << "Unsupported form: " << form << std::endl;
        exit(0);
        break;
    }

    return nullptr;
}

StringTable::StringTable(char *debugStr) : debugStr(debugStr) {}

StringTable::~StringTable() {}

std::string StringTable::getString(size_t offset) {
    std::string res;
    
    for (int i = offset; debugStr[i] != '\0'; i++) {
        res += debugStr[i];
    }

    return res;
}

DwarfParser::DwarfParser(const char *fileName)
    : ElfParser::ElfParser(fileName) {
    const ElfSectionHeader *debugInfoHeader = getSectionHeader(".debug_info");
    uint8_t *debugInfoCUHeader = elfFileInfo->elfData + debugInfoHeader->offset;
    uint8_t *debugInfoEnd = debugInfoCUHeader + debugInfoHeader->size;
    uint8_t *debugAbbrevStart =
        elfFileInfo->elfData + getSectionHeader(".debug_abbrev")->offset;
    uint8_t *debugStrStart =
        elfFileInfo->elfData + getSectionHeader(".debug_str")->offset;

    // Initialize Compile Units
    for (;;) {
        // End of .debug_info section, CU parsing complete
        if ((uintptr_t)debugInfoCUHeader >= ((uintptr_t)debugInfoEnd)) {
            break;
        }

        CompileUnit *compileUnit =
            new CompileUnit(debugInfoCUHeader, debugAbbrevStart, debugStrStart);
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
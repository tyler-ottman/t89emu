#include "DwarfParser.h"

DebugData::DebugData(FormEncoding form) : form(form) {}

DebugData::~DebugData() {}

uint8_t DebugData::decodeUInt8(const uint8_t **data) {
    const uint8_t *bytes = *data;
    uint16_t res = bytes[0];
    (*data)++;
    return res;
}

uint16_t DebugData::decodeUInt16(const uint8_t **data) {
    const uint8_t *bytes = *data;
    uint16_t res = bytes[0] | bytes[1] << 8;
    *data += 2;
    return res;
}

uint32_t DebugData::decodeUInt32(const uint8_t **data) {
    const uint8_t *bytes = *data;
    uint16_t res = bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;
    *data += 4;
    return res;
}

int64_t DebugData::decodeLeb128(const uint8_t **data) {
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

size_t DebugData::decodeULeb128(const uint8_t **data) {
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
    const uint8_t *abbrevData = abbrevTableStart;

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
    size_t code = DebugData::decodeULeb128((const uint8_t **)&byteStream);
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
        DebugData *data = decodeInfo(attEntry->getForm());
        node->addAttribute(attEntry->getName(), data);
    }

    // Next DIE node is child, sibling, or null entry
    // DebugInfoEntry *next = new DebugInfoEntry(
    //     this, dieAbbrevEntry->hasChildren() ? node : node->getParent());
    
    // generateDebugInfo(next);
}

DebugData *CompileUnit::decodeInfo(FormEncoding form) {
    DebugData *dieData = new DebugData(form);
    size_t streamLen = 0;

    switch (form) {
    case DW_FORM_addr: // Read addr_size bytes from stream
        streamLen = compileUnitHeader->getAddressSize();
        break;
    
    case DW_FORM_block: // Read LEB128 length, then block
        streamLen = DebugData::decodeULeb128((const uint8_t **)&byteStream);
        break;
    case DW_FORM_block1: // 1-byte length
        streamLen = DebugData::decodeUInt8((const uint8_t **)&byteStream);
        break;
    case DW_FORM_block2: // 2-byte length
        streamLen = DebugData::decodeUInt16((const uint8_t **)&byteStream);
        break;
    case DW_FORM_block4: // 4-byte length
        streamLen = DebugData::decodeUInt32((const uint8_t**)&byteStream);
        break;

    // Read N-Byte integer
    case DW_FORM_data1: 
    case DW_FORM_flag: streamLen = 1; break;

    case DW_FORM_data2: streamLen = 2; break;
    
    case DW_FORM_data4: streamLen = 4; break;
    
    case DW_FORM_data8: streamLen = 8; break;
    
    // Read LEB128
    case DW_FORM_sdata: {
        int64_t value = DebugData::decodeLeb128((const uint8_t **)&byteStream);
        dieData->write((uint8_t *)&value, sizeof(int64_t));
    }
        break;

    

    // Read string
    case DW_FORM_string:
        for (streamLen = 0; *byteStream != '\0'; streamLen++) {}
        break;

    default:
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
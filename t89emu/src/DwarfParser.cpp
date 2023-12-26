#include "DwarfParser.h"

static size_t decodeLeb128(const uint8_t **data) {
    size_t result = 0;
    size_t shift = 0;
    
    for (;;) {
        result |= (0x7f & **data) << shift;
        if (!(0x80 & *(*data)++)) break;
        shift += 7;
    }

    return result;
}

AttributeEntry::AttributeEntry(size_t name, size_t form, size_t special)
    : name(name), form(form), special(special) {}

AttributeEntry::~AttributeEntry() {}

size_t AttributeEntry::getName() {
    return name;
}

size_t AttributeEntry::getForm() {
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
        size_t dieCode = decodeLeb128(&abbrevData);
        if (dieCode == 0) { // End of Compile Unit
            break;
        }

        size_t dieTag = decodeLeb128(&abbrevData);
        size_t hasChild = decodeLeb128(&abbrevData);
        AbbrevEntry *abbrevEntry = new AbbrevEntry(dieCode, dieTag, hasChild);

        for (;;) {
            size_t name = decodeLeb128(&abbrevData);
            size_t form = decodeLeb128(&abbrevData);
            if (name == 0 && form == 0) { break; }
            size_t special = form == DW_FORM_implicit_const 
                             ? decodeLeb128(&abbrevData) : 0;

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

CompileUnit::CompileUnit(uint8_t *debugInfoCompileUnitHeader,
                         uint8_t *debugAbbrevStart) {
    compileUnitHeader = new CompileUnitHeader(debugInfoCompileUnitHeader);
    abbrevTable = new AbbrevTable(debugAbbrevStart +
                                  compileUnitHeader->getDebugAbbrevOffset());
}

CompileUnit::~CompileUnit() {
    delete compileUnitHeader;
    delete abbrevTable;
}

size_t CompileUnit::getLength() {
    // size of length-field (4 bytes) + Rest of CU Header + DIEs
    return 4 + compileUnitHeader->getUnitLength();
}

DwarfParser::DwarfParser(const char *fileName)
    : ElfParser::ElfParser(fileName) {
    const ElfSectionHeader *debugInfoHeader = getSectionHeader(".debug_info");
    uint8_t *debugInfoCUHeader = elfFileInfo->elfData + debugInfoHeader->offset;
    uint8_t *debugInfoEnd = debugInfoCUHeader + debugInfoHeader->size;
    uint8_t *debugAbbrevStart =
        elfFileInfo->elfData + getSectionHeader(".debug_abbrev")->offset;

    // Initialize Compile Units
    for (;;) {
        // End of .debug_info section, CU parsing complete
        if ((uintptr_t)debugInfoCUHeader >= ((uintptr_t)debugInfoEnd)) {
            break;
        }

        CompileUnit *compileUnit =
            new CompileUnit(debugInfoCUHeader, debugAbbrevStart);
        compileUnits.push_back(compileUnit);

        // Point to next CU Header
        debugInfoCUHeader += compileUnit->getLength();
    }
}

DwarfParser::~DwarfParser() {
    
}
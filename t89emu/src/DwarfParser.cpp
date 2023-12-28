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
        size_t dieCode = decodeLeb128(&abbrevData);
        if (dieCode == 0) { // End of Compile Unit
            break;
        }

        size_t dieTag = decodeLeb128(&abbrevData);
        size_t hasChild = decodeLeb128(&abbrevData);
        AbbrevEntry *abbrevEntry = new AbbrevEntry(dieCode, dieTag, hasChild);

        for (;;) {
            AttributeEncoding name =
                (AttributeEncoding)decodeLeb128(&abbrevData);
            FormEncoding form = (FormEncoding)decodeLeb128(&abbrevData);
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

void DebugInfoEntry::addAttribute(AttributeEncoding encoding, size_t field) {
    attributes.insert({encoding, field});
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

CompileUnit::CompileUnit(uint8_t *debugInfoCUHeader,
                         uint8_t *debugAbbrevStart) {
    compileUnitHeader = new CompileUnitHeader(debugInfoCUHeader);
    abbrevTable = new AbbrevTable(debugAbbrevStart +
                                  compileUnitHeader->getDebugAbbrevOffset());
    byteStream =
        debugInfoCUHeader + sizeof(CompileUnitHeader::FullCompileUnitHeader);
    root = new DebugInfoEntry(this, nullptr);
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
    size_t code = decodeLeb128((const uint8_t **)&byteStream);
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
        size_t field = decodeInfo(attEntry->getForm());
        node->addAttribute(attEntry->getName(), field);
    }

    // Next DIE node is child, sibling, or null entry
    // DebugInfoEntry *next = new DebugInfoEntry(
    //     this, dieAbbrevEntry->hasChildren() ? node : node->getParent());
    
    // generateDebugInfo(next);
}

size_t CompileUnit::decodeInfo(FormEncoding form) {
    switch (form) {

    default:
        break;
    }

    return 0;
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
    for (CompileUnit *compileUnit : compileUnits) {
        delete compileUnit;
    }
    compileUnits.clear();
}
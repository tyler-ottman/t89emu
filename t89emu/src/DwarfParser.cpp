#include "DwarfParser.h"

DwarfParser::DwarfParser(const char *fileName) : ElfParser::ElfParser(fileName) {
    debugInfo = getSectionHeader(".debug_info");
    ASSERT(debugInfo, ".debug_info not found\n");

    debugAbbrev = getSectionHeader(".debug_abbrev");
    ASSERT(debugAbbrev, ".debug_abbrev not found\n");

    generateAbbrevTable();
}

DwarfParser::~DwarfParser() {

}

void DwarfParser::generateAbbrevTable() {
    const uint8_t *abbrevData = elfFileInfo->elfData + debugAbbrev->offset;

    for (;;) {
        if (((uintptr_t)abbrevData -
             (uintptr_t)(elfFileInfo->elfData + debugAbbrev->offset)) >=
            debugAbbrev->size) {
            break;
        }

        AbbrevEntry *abbrevEntry = new AbbrevEntry();

        abbrevEntry->code = decodeLeb128(&abbrevData);
        if (abbrevEntry->code == 0) { // End of Compile Unit Abbreviation
            delete abbrevEntry;
            continue;
        }

        abbrevEntry->tag = decodeLeb128(&abbrevData);
        abbrevEntry->hasChild = *abbrevData++;

        for (;;) {
            size_t name = decodeLeb128(&abbrevData);
            size_t form = decodeLeb128(&abbrevData);
            if (name == 0 && form == 0) break;

            AttributeEntry *atEntry = new AttributeEntry();
            atEntry->name = name;
            atEntry->form = form;
            
            if (atEntry->form == DW_FORM_implicit_const) {
                atEntry->special = decodeLeb128(&abbrevData);
            }

            abbrevEntry->atEntries.push_back(atEntry);
        }

        abbrevTable.insert({abbrevEntry->code, abbrevEntry});
    }
}

size_t DwarfParser::decodeLeb128(const uint8_t **data) {
    size_t result = 0;
    size_t shift = 0;
    
    for (;;) {
        result |= (0x7f & **data) << shift;
        if (!(0x80 & *(*data)++)) break;
        shift += 7;
    }

    return result;
}
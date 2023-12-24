#ifndef DWARFPARSER_H
#define DWARFPARSER_H

#include <map>

#include "DwarfEncodings.h"
#include "ElfParser.h"

struct BaseUnitHeader {
    uint32_t unitLength;
    uint16_t version;
    uint8_t unitType;
    uint8_t addressSize;
    uint32_t debugAbbrevOffset;
};

struct CompileUnitHeader {
    struct BaseUnitHeader;
};

struct AttributeEntry {
    size_t name;
    size_t form;
    size_t special;
};

struct AbbrevEntry {
    size_t code;
    size_t tag;
    bool hasChild;
    std::vector<AttributeEntry *> atEntries;
};

class DwarfParser : public ElfParser {
public:
    DwarfParser(const char *fileName);
    ~DwarfParser();

    void generateAbbrevTable(void);
    
private:
    size_t decodeLeb128(const uint8_t **data);

    std::map<size_t, AbbrevEntry *> abbrevTable;

    const ElfSectionHeader *debugAbbrev;
    const ElfSectionHeader *debugInfo;
};

#endif // DWARFPARSER_H
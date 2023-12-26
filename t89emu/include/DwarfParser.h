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

class AttributeEntry {
public:
    AttributeEntry(size_t name, size_t form, size_t special);
    ~AttributeEntry();

    size_t getName(void);
    size_t getForm(void);
    size_t getSpecial(void);

private:
    size_t name;
    size_t form;
    size_t special;
};

class AbbrevEntry {
public:
    AbbrevEntry(size_t dieCode, size_t dieTag, bool hasChild);
    ~AbbrevEntry();

    void addAttributeEntry(AttributeEntry *attEntry);

    size_t getNumAttributes(void);
    AttributeEntry *getAttributeEntry(size_t index);
    size_t getDieCode(void);
    size_t getDieTag(void);
    bool hasChildren(void);

private:
    size_t dieCode;
    size_t dieTag;
    bool hasChild;

    std::vector<AttributeEntry *> attEntries;
};

class AbbrevTable {
public:
    AbbrevTable(uint8_t *abbrevTableStart);
    ~AbbrevTable();

    AbbrevEntry *getAbbrevEntry(size_t dieCode);

private:
    std::map<size_t, AbbrevEntry *> abbrevEntries;
};

// Compile Unit Header in .debug_info
class CompileUnitHeader {
public:
    CompileUnitHeader(uint8_t *debugInfoCompileUnitHeader);
    ~CompileUnitHeader();

    uint32_t getUnitLength(void);
    uint16_t getUnitVersion(void);
    uint8_t getUnitType(void);
    uint8_t getAddressSize(void);
    uint32_t getDebugAbbrevOffset(void);

private:
    struct FullCompileUnitHeader {
        struct BaseUnitHeader base;
    };

    FullCompileUnitHeader header;
};

class CompileUnit {
public:
    CompileUnit(uint8_t *debugInfoCompileUnitHeader, uint8_t *debugAbbrevStart);
    ~CompileUnit();

    size_t getLength(void);

private:
    CompileUnitHeader *compileUnitHeader;

    // Compile Unit Abbreviation Table
    AbbrevTable *abbrevTable;
};

class DwarfParser : public ElfParser {
public:
    DwarfParser(const char *fileName);
    ~DwarfParser();
    
private:
    std::vector<CompileUnit *> compileUnits;
};

#endif // DWARFPARSER_H
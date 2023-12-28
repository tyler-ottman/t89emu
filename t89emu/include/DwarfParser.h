#ifndef DWARFPARSER_H
#define DWARFPARSER_H

#include <map>

#include "DwarfEncodings.h"
#include "ElfParser.h"

class CompileUnit;
class DebugInfoEntry;

struct BaseUnitHeader {
    uint32_t unitLength;
    uint16_t version;
    uint8_t unitType;
    uint8_t addressSize;
    uint32_t debugAbbrevOffset;
};

class AttributeEntry {
public:
    AttributeEntry(AttributeEncoding name, FormEncoding form, size_t special);
    ~AttributeEntry();

    AttributeEncoding getName(void);
    FormEncoding getForm(void);
    size_t getSpecial(void);

private:
    AttributeEncoding name;
    FormEncoding form;
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

    struct FullCompileUnitHeader {
        struct BaseUnitHeader base;
    };

private:
    FullCompileUnitHeader header;
};

class DebugInfoEntry {
public:
    DebugInfoEntry(CompileUnit *compileUnit, DebugInfoEntry *parent);
    ~DebugInfoEntry();

    void addAttribute(AttributeEncoding encoding, size_t field);

    AbbrevEntry *getAbbrevEntry(void);
    size_t getCode(void);
    DebugInfoEntry *getParent(void);

    void setAbbrevEntry(AbbrevEntry *abbrevEntry);
    void setCode(size_t dieCode);

private:
    CompileUnit *compileUnit;
    
    AbbrevEntry *abbrevEntry;
    std::map<AttributeEncoding, size_t> attributes;

    std::vector<DebugInfoEntry *> children;
    DebugInfoEntry *parent;
    
    size_t code;
    std::string name;
};

class CompileUnit {
public:
    CompileUnit(uint8_t *debugInfoCUHeader, uint8_t *debugAbbrevStart);
    ~CompileUnit();

    AbbrevEntry *getAbbrevEntry(size_t dieCode);
    size_t getAddrSize(void);
    size_t getLength(void);

private:
    // Recursively create Tree DIE structure in memory
    void generateDebugInfo(DebugInfoEntry *node);

    // Given an attribute's form, read bytes from byteStream accordingly
    size_t decodeInfo(FormEncoding form);

    // CU Header for corresponding CU in .debug_info 
    CompileUnitHeader *compileUnitHeader;

    // CU Abbreviation Table
    AbbrevTable *abbrevTable;

    DebugInfoEntry *root;

    // Starts at first byte of first DIE entry within CU
    uint8_t *byteStream;
};

class DwarfParser : public ElfParser {
public:
    DwarfParser(const char *fileName);
    ~DwarfParser();
    
private:
    std::vector<CompileUnit *> compileUnits;
};

#endif // DWARFPARSER_H
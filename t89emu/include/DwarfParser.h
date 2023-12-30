#ifndef DWARFPARSER_H
#define DWARFPARSER_H

#include <map>
#include <string>

#include "DwarfEncodings.h"
#include "ElfParser.h"

class CompileUnit;
class DebugInfoEntry;
class StringTable;

struct BaseUnitHeader {
    uint32_t unitLength;
    uint16_t version;
    uint8_t unitType;
    uint8_t addressSize;
    uint32_t debugAbbrevOffset;
};

class DataStream {
public:
    DataStream(uint8_t *data);
    ~DataStream();

    // For reading data from byteStream
    uint8_t decodeUInt8(void);
    uint16_t decodeUInt16(void);
    uint32_t decodeUInt32(void);
    int64_t decodeLeb128(void);
    size_t decodeULeb128(void);

    const uint8_t *getData(void);

private:
    uint8_t *data;
};

// Data associated with a DIE Attribute Entry
class DebugData {
public:
    DebugData(FormEncoding form);
    ~DebugData();

    void write(uint8_t *buff, size_t len);
    bool isString(void);

    FormEncoding getForm(void);
    uint64_t getUInt(void);
    const char *getString(void);

private:
    std::vector<uint8_t> data;
    FormEncoding form;
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
    AbbrevEntry(size_t dieCode, TagEncoding dieTag, bool hasChild);
    ~AbbrevEntry();

    void addAttributeEntry(AttributeEntry *attEntry);

    size_t getNumAttributes(void);
    AttributeEntry *getAttributeEntry(size_t index);
    size_t getDieCode(void);
    TagEncoding getDieTag(void);
    bool hasChildren(void);

private:
    size_t dieCode;
    TagEncoding dieTag;
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

    DataStream *abbrevData;
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

    void addAttribute(AttributeEncoding encoding, DebugData *data);
    void addChild(DebugInfoEntry *child);
    void printEntry(void);

    AbbrevEntry *getAbbrevEntry(void);
    size_t getCode(void);
    DebugInfoEntry *getParent(void);

    void setAbbrevEntry(AbbrevEntry *abbrevEntry);
    void setCode(size_t dieCode);

private:
    CompileUnit *compileUnit;
    
    AbbrevEntry *abbrevEntry;
    std::map<AttributeEncoding, DebugData *> attributes; // {Attribute, Data}

    std::vector<DebugInfoEntry *> children;
    DebugInfoEntry *parent;
    
    size_t code;
    std::string name;
};

class CompileUnit {
public:
    CompileUnit(uint8_t *debugInfoCUHeader, uint8_t *debugAbbrevStart,
                uint8_t *debugStrStart, uint8_t *debugLineStrStart);
    ~CompileUnit();

    AbbrevEntry *getAbbrevEntry(size_t dieCode);
    size_t getAddrSize(void);
    size_t getLength(void);

private:
    // Recursively create Tree DIE structure in memory
    DebugInfoEntry *generateDebugInfo(DebugInfoEntry *node);

    // Given an attribute's form, read bytes from byteStream accordingly
    DebugData *decodeInfo(AttributeEntry *entry);

    void printDebugInfoEntry(DebugInfoEntry *node);

    // CU Header for corresponding CU in .debug_info 
    CompileUnitHeader *compileUnitHeader;

    // CU Abbreviation Table
    AbbrevTable *abbrevTable;

    // String Tables (.debug_info, .debug_line_str)
    StringTable *debugStr;
    StringTable *debugLineStr;

    DebugInfoEntry *root;

    // Starts at first byte of first DIE entry within CU
    DataStream *debugStream;

    // Starting address of CU within .debug_info
    uintptr_t debugStart;
};

class StringTable {
public:
    StringTable(char *tableStart);
    ~StringTable();

    std::string getString(size_t offset);

private:
    char *tableStart;
};

class DwarfParser : public ElfParser {
public:
    DwarfParser(const char *fileName);
    ~DwarfParser();

private:
    std::vector<CompileUnit *> compileUnits;
};

#endif // DWARFPARSER_H
#ifndef DWARFPARSER_H
#define DWARFPARSER_H

#include <map>
#include <memory>
#include <stack>
#include <string>

#include "DwarfEncodings.h"
#include "ElfParser.h"

class CompileUnit;
class DebugData;
class DebugInfoEntry;
class StringTable;

struct BaseUnitHeader {
    uint32_t unitLength;
    uint16_t version;
    uint8_t unitType;
    uint8_t addressSize;
    uint32_t debugAbbrevOffset;
};

// For processing Dwarf Expressions
class StackMachine {
public:
    StackMachine(void);
    ~StackMachine();

    uint32_t processExpression(DebugData *expr);
    OperationEncoding getType(void);

private:
    std::stack<uint32_t> stack;
    OperationEncoding opType; // FIrst operation parsed in expression
};

class DataStream {
public:
    DataStream(const uint8_t *data, size_t streamLen);
    DataStream(const uint8_t *data);
    ~DataStream();

    // For reading data from byteStream
    uint8_t decodeUInt8(void);
    uint16_t decodeUInt16(void);
    uint32_t decodeUInt32(void);
    int64_t decodeLeb128(void);
    size_t decodeULeb128(void);

    bool isStreamable(void);
    const uint8_t *getData(void);

private:
    const uint8_t *data;
    size_t index;
    size_t streamLen;
};

// Data associated with a DIE Attribute Entry
class DebugData {
public:
    DebugData(FormEncoding form);
    ~DebugData();

    void write(uint8_t *buff, size_t len);
    bool isString(void);

    const uint8_t *getData(void);
    FormEncoding getForm(void);
    uint64_t getUInt(void);
    size_t getLen(void);
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
    size_t getHeaderLen(void);

private:
    struct FullCompileUnitHeader {
        struct BaseUnitHeader base;
    };

    FullCompileUnitHeader header;
};

class Variable {
public:
    Variable(DebugInfoEntry *debugEntry);
    ~Variable();

    uint32_t getLocation(void);
    std::string& getName(void);

private:
    void processLocation(void);

    DebugInfoEntry *debugEntry;

    std::string name;
    
    OperationEncoding locType;
    uint32_t location;
};

class Scope {
public:
    Scope(DebugInfoEntry *debugEntry, Scope *parent);
    ~Scope();

    void addScope(Scope *childScope);
    void addVariable(Variable *childVar);
    void printScopes(int depth);

    const char *getName(void);

    // void getLocalVariables(std::vector<Variable *>& res);
    bool isPcInRange(uint32_t pc);

private:
    std::string name;

    Scope *parent;
    std::vector<Scope *> scopes;
    std::vector<Variable *> variables;

    uint32_t lowPc;
    uint32_t highPc;
};

class DebugInfoEntry {
public:
    DebugInfoEntry(CompileUnit *compileUnit, DebugInfoEntry *parent);
    ~DebugInfoEntry();

    void addAttribute(AttributeEncoding encoding, DebugData *data);
    void addChild(DebugInfoEntry *child);
    void printEntry(void);

    AbbrevEntry *getAbbrevEntry(void);
    DebugData *getAttribute(AttributeEncoding attribute);
    size_t getCode(void);
    TagEncoding getTag(void);
    size_t getNumChildren(void);
    DebugInfoEntry *getChild(size_t index);
    DebugInfoEntry *getParent(void);

    void setAbbrevEntry(AbbrevEntry *abbrevEntry);
    void setCode(size_t dieCode);

    bool isScope(void);
    bool isType(void);
    bool isVariable(void);

private:
    CompileUnit *compileUnit;
    
    AbbrevEntry *abbrevEntry;
    std::map<AttributeEncoding, DebugData *> attributes; // {Attribute, Data}

    std::vector<DebugInfoEntry *> children;
    DebugInfoEntry *parent;

    size_t code;
    // std::string name;
};

class CompileUnit {
public:
    CompileUnit(uint8_t *debugInfoCUHeader, uint8_t *debugAbbrevStart,
                uint8_t *debugStrStart, uint8_t *debugLineStrStart);
    ~CompileUnit();

    void generateScopes(void);
    void printScopes(void);

    AbbrevEntry *getAbbrevEntry(size_t dieCode);
    size_t getAddrSize(void);
    size_t getLength(void);
    Scope *getScope(uint32_t pc);

    bool isPcInRange(uint32_t pc);

private:
    // Recursively create Tree DIE structure in memory
    DebugInfoEntry *generateDebugInfo(DebugInfoEntry *node);

    // Recursively generate scopes, and identify variables within scopes
    Scope *generateScopes(DebugInfoEntry *node, Scope *parent);

    // Given an attribute's form, read bytes from byteStream accordingly
    DebugData *decodeInfo(AttributeEntry *entry);

    // CU Header for corresponding CU in .debug_info 
    CompileUnitHeader *compileUnitHeader;
    size_t compileUnitLen; // Compile Unit Header + payload
    size_t headerLen;

    // CU Abbreviation Table
    AbbrevTable *abbrevTable;

    // String Tables (.debug_info, .debug_line_str)
    StringTable *debugStr;
    StringTable *debugLineStr;

    DebugInfoEntry *root;
    Scope *rootScope;

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

    Scope *getScope(uint32_t pc);

private:
    std::vector<CompileUnit *> compileUnits;
};

#endif // DWARFPARSER_H

#ifndef DWARFPARSER_H
#define DWARFPARSER_H

#include <fstream>
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

    void setOffset(size_t offset); // Offset from center

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
    OperationEncoding getType(void);
    DebugData *getAttribute(AttributeEncoding attribute);

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
    void getLocalVariables(std::vector<Variable *> &res, uint32_t pc, uint line);
    void getGlobalVariables(std::vector<Variable *> &res, uint32_t pc, uint line);
    bool isLocalScope(uint32_t pc);
    bool isPcInRange(uint32_t pc);

private:
    void getVariablesAboveLine(std::vector<Variable *> &ret, uint line);

    std::string name;

    Scope *parent;
    std::vector<Scope *> scopes;
    std::vector<Variable *> variables;

    uint32_t lowPc;
    uint32_t highPc;
};

class SourceInfo {
public:
    SourceInfo(std::string &path);
    ~SourceInfo();

    std::string &getPath(void);
    std::string &getName(void);
    std::vector<std::string> &getLines(void);

private:
    std::string path;
    std::string name;
    std::vector<std::string> lines;
};

class LineNumberInfo {
public:
    LineNumberInfo(CompileUnit *cu, uint8_t *debugLineStart);
    ~LineNumberInfo();

    void generateFileFormatInfo(uint8_t &numEntries,
        std::vector<std::pair<ContentEncoding, FormEncoding>> &formatEntries);
    void generateFileInformation(uint &numEntries,
        std::vector<std::map<ContentEncoding, DebugData *>> &fileEntries,
        std::vector<std::pair<ContentEncoding, FormEncoding>> &fileFormat);

    // get total length of the line number information for this compile unit
    size_t getLength(void);
    std::vector<SourceInfo *> &getSourceInfo(void);
    uint getLineNumberAtPc(uint32_t pc);
    std::string &getSourceNameAtPc(uint32_t pc);

    static bool containsPath(std::vector<SourceInfo *> &sources,
                             std::string &path);

private:
    struct ProgramHeader {
        uint32_t unitLength;
        uint16_t version;
        uint8_t addressSize;
        uint8_t segmentSelectorSize;
        uint32_t headerLength;
        uint8_t minimumInstructionLength;
        uint8_t maximumOperationsPerInstruction;
        uint8_t defaultIsStmt;
        int8_t lineBase;
        uint8_t lineRange;
        uint8_t opcodeBase;
        std::vector<uint8_t> standardOpcodeLengths;
        uint8_t directoryEntryFormatCount;
        std::vector<std::pair<ContentEncoding, FormEncoding>>
            directoryEntryFormat;
        uint directoriesCount;
        std::vector<std::map<ContentEncoding, DebugData *>> directories;
        uint8_t fileNameEntryFormatCount;
        std::vector<std::pair<ContentEncoding, FormEncoding>>
            fileNameEntryFormat;
        uint fileNamesCount;
        std::vector<std::map<ContentEncoding, DebugData *>> fileNames;
    };

    struct LineEntry {
        LineEntry(uint32_t address, uint line, uint column, uint file,
                  bool isStmt, bool prologueEnd, bool epilogueBegin)
            : address(address), line(line), column(column), file(file),
              isStmt(isStmt), prolgueEnd(prologueEnd),
              epilogueBegin(epilogueBegin) {}
        uint32_t address;
        uint line;
        uint column;
        uint file;
        bool isStmt; // flags for setting breakpoints
        bool prolgueEnd;
        bool epilogueBegin;
    };

    void clearRegisters(void); // End of every sequence
    void generateFilePaths(void);
    void generateLineNumberMatrix(void);
    void parseProgramHeader(void);

    uint getNewAddress(uint operationAdvance);
    uint getNewOpIndex(uint opeartionAdvance);
    LineEntry *getLineEntryAtPc(uint32_t pc);

    ProgramHeader infoHeader;
    std::vector<LineEntry *> lineMatrix;
    std::vector<SourceInfo *> sourceInfo;
    DataStream *stream; // .debug_line stream
    CompileUnit *compileUnit;

    uint32_t address;
    uint opIndex;
    uint file;
    uint line;
    uint column;
    bool isStmt;
    bool basicBlock;
    bool endSequence;
    bool prologueEnd;
    bool epilogueBegin;
    uint isa;
    uint discriminator;
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
                uint8_t *debugStrStart, uint8_t *debugLineStrStart,
                uint8_t *debugLineStart);
    ~CompileUnit();

    void generateScopes(void);
    void printScopes(void);

    // Given an attribute's form, read bytes from byteStream accordingly
    DebugData *decodeInfo(AttributeEntry *entry, DataStream *stream);

    AbbrevEntry *getAbbrevEntry(size_t dieCode);
    size_t getAddrSize(void);
    size_t getDebugInfoLength(void); // Returns size of .debug_info section for corresponding CU
    size_t getDebugLineLength(void); // Returns size of .debug_line section for corresponding CU
    Scope *getScope(uint32_t pc);
    const char *getUnitName(void);
    const char *getUnitDir(void);
    std::vector<SourceInfo *> &getSourceInfo(void);
    uint getLineNumberAtPc(uint32_t pc);
    std::string &getSourceNameAtPc(uint32_t pc);
    void getLocalVariables(std::vector<Variable *> &variables, uint32_t pc,
                           uint line);
    void getGlobalVariables(std::vector<Variable *> &variables, uint32_t pc,
                            uint line);

    bool isPcInRange(uint32_t pc);

private:
    // Recursively create Tree DIE structure in memory
    DebugInfoEntry *generateDebugInfo(DebugInfoEntry *node);

    // Recursively generate scopes, and identify variables within scopes
    Scope *generateScopes(DebugInfoEntry *node, Scope *parent);

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

    LineNumberInfo *debugLine;
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
    CompileUnit *getCompileUnitAtPc(uint32_t pc);
    size_t getNumCompileUnits(void);
    std::vector<SourceInfo *> &getSourceInfo(void);
    uint getLineNumberAtPc(uint32_t pc);
    std::string &getSourceNameAtPc(uint32_t pc);
    void getLocalVariables(std::vector<Variable *> &variables, uint32_t pc,
                           uint line);
    void getGlobalVariables(std::vector<Variable *> &variables, uint32_t pc,
                            uint line);

   private:
    std::vector<CompileUnit *> compileUnits;

    // Source File Information
    std::vector<SourceInfo *> sourceInfo;
};

#endif // DWARFPARSER_H

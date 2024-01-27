#pragma once

#include <stddef.h>
#include <stdint.h>

#include "Dwarf/DebugInfoEntry.h"
#include "Dwarf/LineNumberInfo.h"
#include "Dwarf/Scope.h"
#include "Dwarf/StringTable.h"
#include "Dwarf/Variable.h"

class AbbrevEntry;
class AbbrevTable;
class AttributeEntry;
class LineNumberInfo;
class Scope;
class SourceInfo;

struct BaseUnitHeader {
    uint32_t unitLength;
    uint16_t version;
    uint8_t unitType;
    uint8_t addressSize;
    uint32_t debugAbbrevOffset;
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

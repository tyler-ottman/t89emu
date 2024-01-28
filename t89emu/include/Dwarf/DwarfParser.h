#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "Dwarf/CallFrameInfo.h"
#include "Dwarf/CompileUnit.h"
#include "Dwarf/DataType.h"
#include "Dwarf/DwarfEncodings.h"
#include "Dwarf/LineNumberInfo.h"
#include "Dwarf/Scope.h"
#include "Dwarf/Variable.h"
#include "ElfParser.h"
#include "Mcu.h"

class DataType;
class Scope;
class SourceInfo;
struct VarInfo;

class DwarfParser : public ElfParser {
public:
    DwarfParser(const char *fileName);
    ~DwarfParser();

    void generateTypes(void);
    void addTypeEntry(size_t offset, DataType *dateType);

    Scope *getScope(uint32_t pc);
    DataType *getTypeEntry(size_t offset);
    CompileUnit *getCompileUnitAtPc(uint32_t pc);
    size_t getNumCompileUnits(void);
    std::vector<SourceInfo *> &getSourceInfo(void);
    uint getLineNumberAtPc(uint32_t pc);
    std::string &getSourceNameAtPc(uint32_t pc);
    void getLocalVariables(std::vector<Variable *> &variables, uint32_t pc,
                           uint line);
    void getGlobalVariables(std::vector<Variable *> &variables, uint32_t pc,
                            uint line);
    void getVarInfo(VarInfo &res, bool doUpdate, Variable *var,
                    RegisterFile *regs, uint32_t pc);
    uint8_t *getDebugAbbrevStart(void);
    uint8_t *getDebugFrameStart(void);
    uint8_t *getDebugInfoStart(void);
    uint8_t *getDebugLineStart(void);
    uint8_t *getDebugLineStrStart(void);
    uint8_t *getDebugStrStart(void);

private:
    std::vector<CompileUnit *> compileUnits;

    // Source File Information
    std::vector<SourceInfo *> sourceInfo;

    CallFrameInfo *debugFrame;

    std::unordered_map<size_t, DataType *> typeEntries;

    uint8_t *debugAbbrevStart;
    uint8_t *debugFrameStart;
    uint8_t *debugInfoStart;
    uint8_t *debugLineStart;
    uint8_t *debugLineStrStart;
    uint8_t *debugStrStart;
};

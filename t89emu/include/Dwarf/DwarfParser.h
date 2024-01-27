#ifndef DWARFPARSER_H
#define DWARFPARSER_H

#include <fstream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "Dwarf/CallFrameInfo.h"
#include "Dwarf/CompileUnit.h"
#include "Dwarf/DwarfEncodings.h"
#include "Dwarf/LineNumberInfo.h"
#include "Dwarf/Scope.h"
#include "Dwarf/Variable.h"
#include "ElfParser.h"
#include "Mcu.h"

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
    void getVarInfo(Variable::VarInfo &res, bool doUpdate, Variable *var,
                    RegisterFile *regs, uint32_t pc);

private:
    std::vector<CompileUnit *> compileUnits;

    // Source File Information
    std::vector<SourceInfo *> sourceInfo;

    CallFrameInfo *debugFrame;
};

#endif // DWARFPARSER_H

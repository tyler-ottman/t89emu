#pragma once

#include "Dwarf/CallFrameInfo.h"
#include "Dwarf/DebugInfoEntry.h"
#include "Dwarf/DwarfExpr.h"
#include "Dwarf/Variable.h"
#include "RegisterFile.h"

class DebugInfoEntry;
class StackMachine;

class Variable {
public:
    struct VarInfo {
        bool isValid;

        void *type;
        std::string name;
        uint32_t location; // Todo: multiple locations
        std::string value;
    };

    Variable(DebugInfoEntry *debugEntry);
    ~Variable();

    DebugData *getAttribute(AttributeEncoding attribute);
    DebugInfoEntry *getParentEntry(void);
    void getVarInfo(VarInfo &res, bool doUpdate, RegisterFile *regs,
                    CallFrameInfo *cfi, uint32_t pc);

private:
    bool updateLocation(RegisterFile *regs, CallFrameInfo *cfi, uint32_t pc);
    bool updateValue(RegisterFile *regs);

    // DWARF Expression state
    DebugData *locExpr;
    StackMachine *stack;

    // DWARF Debug Information Entry
    DebugInfoEntry *debugEntry;

    // Variable information
    VarInfo varInfo;
};
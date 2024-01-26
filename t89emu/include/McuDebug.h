#ifndef MCUDEBUG_H
#define MCUDEBUG_H

#include <algorithm>
#include <fstream>
#include <sstream>

#include "DwarfParser.h"
#include "Mcu.h"

// Debug interface between mcu and gui/dwarf/elf parser
class McuDebug {
public:
    static McuDebug *getInstance(const char *elfPath);

    void executeInstructions(uint cycles);
    void stepInstruction(void);
    void stepLine(void);
    void addBreakpoint(uint32_t address);
    void removeBreakpoint(uint32_t address);

    bool isBreakpoint(uint32_t address);

    ClintMemoryDevice *getClintDevice(void);
    RamMemoryDevice *getRamDevice(void);
    RomMemoryDevice *getRomDevice(void);
    VideoMemoryDevice *getVideoDevice(void);
    RegisterFile *getRegisterFileModule(void);
    ProgramCounter *getProgramCounterModule(void);
    Csr *getCsrModule(void);
    ImmediateGenerator *getImmediateGeneratorModule(void);

    std::vector<DisassembledEntry> &getDisassembledCode(void);
    void getLocalVariables(std::vector<Variable *> &variables);
    void getGlobalVariables(std::vector<Variable *> &variables);
    std::vector<SourceInfo *> &getSourceInfo(void);
    uint getLineNumberAtPc(void);
    std::string &getSourceNameAtPc(void);
    void getVarInfo(Variable::VarInfo &res, bool doUpdate, Variable *var);

private:
    McuDebug(const char *elfPath);
    ~McuDebug();

    Mcu *mcu; // Emulated MCU

    // Debugging information
    DwarfParser *dwarfParser;
    ElfParser *elfParser;
    uint latestSourceLine;
    std::vector<uint32_t> breakpoints;

    static McuDebug *instance;
};

#endif // MCUDEBUG_H
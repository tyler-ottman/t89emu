#ifndef MCUDEBUG_H
#define MCUDEBUG_H

#include <fstream>
#include <sstream>

#include "DwarfParser.h"
#include "Mcu.h"

// Debug interface between mcu and gui/dwarf/elf parser
class McuDebug {
public:
    static McuDebug *getInstance(const char *elfPath);

    void stepInstruction(void);
    void stepLine(void);

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

private:
    McuDebug(const char *elfPath);
    ~McuDebug();

    Mcu *mcu;

    DwarfParser *dwarfParser;
    ElfParser *elfParser;

    static McuDebug *instance;
};

#endif // MCUDEBUG_H
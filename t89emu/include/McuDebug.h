#ifndef MCUDEBUG_H
#define MCUDEBUG_H

#include <fstream>
#include <sstream>

#include "DwarfParser.h"
#include "Mcu.h"

// Source-Level Information
struct SourceFileInfo {
    std::string path;
    std::string name;
    std::vector<std::string> lines;
};

// Debug interface between mcu and gui/dwarf/elf parser
class McuDebug {
public:
    static McuDebug *getInstance(const char *elfPath);

    void nextInstruction(void);

    ClintMemoryDevice *getClintDevice(void);
    RamMemoryDevice *getRamDevice(void);
    RomMemoryDevice *getRomDevice(void);
    VideoMemoryDevice *getVideoDevice(void);
    RegisterFile *getRegisterFileModule(void);
    ProgramCounter *getProgramCounterModule(void);
    Csr *getCsrModule(void);
    ImmediateGenerator *getImmediateGeneratorModule(void);

    std::vector<DisassembledEntry> &getDisassembledCode(void);
    std::vector<SourceFileInfo *> &getSourceFileInfo(void);
    void getLocalVariables(std::vector<Variable *> &variables);
    void getGlobalVariables(std::vector<Variable *> &variables);

private:
    McuDebug(const char *elfPath);
    ~McuDebug();

    void initSourceInfo(void);

    Mcu *mcu;

    DwarfParser *dwarfParser;
    ElfParser *elfParser;

    // Source File Information
    std::vector<SourceFileInfo *> fileInfo;

    static McuDebug *instance;
};

#endif // MCUDEBUG_H
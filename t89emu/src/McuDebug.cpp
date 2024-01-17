#include "McuDebug.h"

McuDebug *McuDebug::instance = nullptr;

McuDebug *McuDebug::getInstance(const char *elfPath) {
    if (instance) {
        return instance;
    }

    instance = new McuDebug(elfPath);

    return instance;
}

void McuDebug::executeInstructions(uint cycles) {
    ProgramCounter *pcModule = mcu->getProgramCounterModule();

    for (; cycles && !isBreakpoint(pcModule->getPc()); cycles--) {
        mcu->nextInstruction();
    }

    // if (isBreakpoint(pcModule->getPc())) {

    // }
}

void McuDebug::stepInstruction() {
    mcu->nextInstruction();
    
    // If new line reached, update latestSourceLine
    uint curLine = dwarfParser->getLineNumberAtPc(
        mcu->getProgramCounterModule()->getPc());
    latestSourceLine = curLine ? curLine : latestSourceLine;
}

void McuDebug::stepLine() {
    uint startPc = mcu->getProgramCounterModule()->getPc();
    uint curPc, curLine;

    // Keep executing instructions until current line changes
    // Or if pc repeats when stepping through a for loop
    do {
        mcu->nextInstruction();
        curPc = mcu->getProgramCounterModule()->getPc();
        curLine = dwarfParser->getLineNumberAtPc(curPc);
    } while ((curLine == 0 || curLine == latestSourceLine) && startPc != curPc);

    latestSourceLine = curLine;
}

void McuDebug::addBreakpoint(uint32_t address) {
    if (std::find(breakpoints.begin(), breakpoints.end(), address) ==
        breakpoints.end()) {  // Breakpoint at address not found, add it
        breakpoints.push_back(address);
    }
}

void McuDebug::removeBreakpoint(uint32_t address) {
    for (size_t i = 0; i < breakpoints.size(); i++) {
        if (address == breakpoints[i]) {
            breakpoints.erase(breakpoints.begin() + i);
            break;
        }
    }
}

bool McuDebug::isBreakpoint(uint32_t address) {
    return std::find(breakpoints.begin(), breakpoints.end(), address) !=
           breakpoints.end();
}

ClintMemoryDevice *McuDebug::getClintDevice() { return mcu->getClintDevice(); }

RamMemoryDevice *McuDebug::getRamDevice() { return mcu->getRamDevice(); }

RomMemoryDevice *McuDebug::getRomDevice() { return mcu->getRomDevice(); }

VideoMemoryDevice *McuDebug::getVideoDevice() { return mcu->getVideoDevice(); }

RegisterFile *McuDebug::getRegisterFileModule() {
    return mcu->getRegisterFileModule();
}

ProgramCounter *McuDebug::getProgramCounterModule() {
    return mcu->getProgramCounterModule();
}

Csr *McuDebug::getCsrModule() { return mcu->getCsrModule(); }

ImmediateGenerator *McuDebug::getImmediateGeneratorModule() {
    return mcu->getImmediateGeneratorModule();
}

std::vector<DisassembledEntry> &McuDebug::getDisassembledCode() {
    return elfParser->getDisassembledCode();
}

std::vector<SourceInfo *> &McuDebug::getSourceInfo() {
    return dwarfParser->getSourceInfo();
}

void McuDebug::getLocalVariables(std::vector<Variable *> &variables) {
    ProgramCounter *pcModule = mcu->getProgramCounterModule();
    dwarfParser->getLocalVariables(variables, pcModule->getPc(),
                                   latestSourceLine);
}

void McuDebug::getGlobalVariables(std::vector<Variable *> &variables) {
    ProgramCounter *pcModule = mcu->getProgramCounterModule();
    dwarfParser->getGlobalVariables(variables, pcModule->getPc(),
                                    latestSourceLine);
}

uint McuDebug::getLineNumberAtPc() {
    return latestSourceLine;
}

std::string &McuDebug::getSourceNameAtPc() {
    return dwarfParser->getSourceNameAtPc(
        mcu->getProgramCounterModule()->getPc());
}

uint32_t McuDebug::getVarLocation(Variable *var) {
    return dwarfParser->getVarLocation(var, mcu->getRegisterFileModule(),
                                       mcu->getProgramCounterModule()->getPc());
}

McuDebug::McuDebug(const char *elfPath) {
    elfParser = new ElfParser(elfPath);
    dwarfParser = new DwarfParser(elfPath);

    mcu = new Mcu(elfParser->getRomStart(), elfParser->getRamStart(),
                  elfParser->getEntryPc());

    elfParser->flashRom(mcu->getRomDevice());

    latestSourceLine = dwarfParser->getLineNumberAtPc(
        mcu->getProgramCounterModule()->getPc());
}

McuDebug::~McuDebug() {}

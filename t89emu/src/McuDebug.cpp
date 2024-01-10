#include "McuDebug.h"

McuDebug *McuDebug::instance = nullptr;

McuDebug *McuDebug::getInstance(const char *elfPath) {
    if (instance) {
        return instance;
    }

    instance = new McuDebug(elfPath);

    return instance;
}

void McuDebug::stepInstruction() { mcu->nextInstruction(); }

void McuDebug::stepLine() {
    uint startPc = mcu->getProgramCounterModule()->getPc();
    uint startLine = dwarfParser->getLineNumberAtPc(startPc);
    uint curPc, curLine;

    // Keep executing instructions until current line changes
    // Or if pc repeats when stepping through a for loop
    do {
        mcu->nextInstruction();
        curPc = mcu->getProgramCounterModule()->getPc();
        curLine = dwarfParser->getLineNumberAtPc(curPc);
    } while ((curLine == 0 || curLine == startLine) && startPc != curPc);
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

void McuDebug::getLocalVariables(std::vector<Variable *> &variables) {}
void McuDebug::getGlobalVariables(std::vector<Variable *> &variables) {}

uint McuDebug::getLineNumberAtPc() {
    return dwarfParser->getLineNumberAtPc(
        mcu->getProgramCounterModule()->getPc());
}

std::string &McuDebug::getSourceNameAtPc() {
    return dwarfParser->getSourceNameAtPc(
        mcu->getProgramCounterModule()->getPc());
}

McuDebug::McuDebug(const char *elfPath) {
    elfParser = new ElfParser(elfPath);
    dwarfParser = new DwarfParser(elfPath);

    mcu = new Mcu(elfParser->getRomStart(), elfParser->getRamStart(),
                  elfParser->getEntryPc());

    elfParser->flashRom(mcu->getRomDevice());
}

McuDebug::~McuDebug() {}
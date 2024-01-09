#include "McuDebug.h"

McuDebug *McuDebug::instance = nullptr;

McuDebug *McuDebug::getInstance(const char *elfPath) {
    if (instance) {
        return instance;
    }

    instance = new McuDebug(elfPath);

    return instance;
}

void McuDebug::nextInstruction() { mcu->nextInstruction(); }

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

std::vector<SourceFileInfo *> &McuDebug::getSourceFileInfo() {
    return fileInfo;
}

void McuDebug::getLocalVariables(std::vector<Variable *> &variables) {}
void McuDebug::getGlobalVariables(std::vector<Variable *> &variables) {}

McuDebug::McuDebug(const char *elfPath) {
    elfParser = new ElfParser(elfPath);
    dwarfParser = new DwarfParser(elfPath);

    mcu = new Mcu(elfParser->getRomStart(), elfParser->getRamStart(),
                  elfParser->getEntryPc());

    elfParser->flashRom(mcu->getRomDevice());

    initSourceInfo();
}

McuDebug::~McuDebug() {}

void McuDebug::initSourceInfo() {
    // Initialize Source Code Viewer
    for (size_t i = 0; i < dwarfParser->getNumCompileUnits(); i++) {
        CompileUnit *cu = dwarfParser->getCompileUnit(i);
        SourceFileInfo *sourceInfo = new SourceFileInfo;
        
        // Full path to source file
        std::stringstream stream;
        stream << cu->getUnitDir() << "/" << cu->getUnitName();
        sourceInfo->path = stream.str();

        // Source file name
        size_t found = sourceInfo->path.find_last_of("/");
        sourceInfo->name = sourceInfo->path.substr(++found);

        // Source file bytes
        std::ifstream fs;
        fs.open(sourceInfo->path, std::ifstream::in);
        if (!fs.is_open()) {
            printf("Could not open file: %s\n", sourceInfo->name.c_str());
            exit(EXIT_FAILURE);
        }

        std::string line;
        while (std::getline(fs, line)) {
            sourceInfo->lines.push_back(line);
        }

        fs.close();
        fileInfo.push_back(sourceInfo);
    }
}
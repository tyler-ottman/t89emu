#include "Dwarf/DwarfParser.h"

DwarfParser::DwarfParser(const char *fileName)
    : ElfParser::ElfParser(fileName) {
    const ElfSectionHeader *debugInfoHeader = getSectionHeader(".debug_info");
    const ElfSectionHeader *debugAbbrevHeader =
        getSectionHeader(".debug_abbrev");
    const ElfSectionHeader *debugStrHeader = getSectionHeader(".debug_str");
    const ElfSectionHeader *debugLineStrHeader =
        getSectionHeader(".debug_line_str");
    const ElfSectionHeader *debugLineHeader = getSectionHeader(".debug_line");
    const ElfSectionHeader *debugFrameHeader = getSectionHeader(".debug_frame");

    uint8_t *fileStart = elfFileInfo->elfData;

    debugInfoStart = fileStart + debugInfoHeader->offset;
    debugAbbrevStart = fileStart + debugAbbrevHeader->offset;
    debugStrStart = fileStart + debugStrHeader->offset;
    debugLineStrStart = fileStart + debugLineStrHeader->offset;
    debugLineStart = fileStart + debugLineHeader->offset;
    debugFrameStart = fileStart + debugFrameHeader->offset;

    // Initialize Compile Units and Debug Information Tree
    uint8_t *debugInfoOffset = debugInfoStart;
    for (;;) {
        // End of .debug_info section, CU parsing complete
        uint8_t *debugInfoEnd = debugInfoStart + debugInfoHeader->size;
        if ((uintptr_t)debugInfoOffset >= ((uintptr_t)debugInfoEnd)) {
            break;
        }

        CompileUnit *compileUnit = new CompileUnit(this, debugInfoOffset);
        compileUnits.push_back(compileUnit);
        
        compileUnit->generateScopes();
        // compileUnit->printScopes();

        // Point to next .debug_info CU Header
        debugInfoOffset += compileUnit->getDebugInfoLength();

        // Point to next .debug_line CU header
        debugLineStart += compileUnit->getDebugLineLength();
    }

    // Initialize Call Frame Information
    debugFrame = new CallFrameInfo(debugFrameStart, debugFrameStart +
                                   debugFrameHeader->size);

    // Generate types
    generateTypes();
}

DwarfParser::~DwarfParser() {
    for (CompileUnit *compileUnit : compileUnits) {
        delete compileUnit;
    }
    compileUnits.clear();
}

void DwarfParser::generateTypes() {
    for (CompileUnit *cu : compileUnits) {
        cu->generateTypes();
    }
}

void DwarfParser::addTypeEntry(size_t offset, DataType *dataType) {
    if (typeEntries.find(offset) == typeEntries.end()) {
        typeEntries.insert({offset, dataType});
    }
}

DataType *DwarfParser::getTypeEntry(size_t offset) {
    return typeEntries.find(offset) != typeEntries.end() ?
        typeEntries[offset] : nullptr;
}

Scope *DwarfParser::getScope(uint32_t pc) {
    for (CompileUnit *unit : compileUnits) {
        if (unit->isPcInRange(pc)) {
            // return unit->getScope(pc);
        }
    }
    return nullptr;
}

CompileUnit *DwarfParser::getCompileUnitAtPc(uint32_t pc) {
    // for (CompileUnit *compileUnit : compileUnits) {
    //     if (compileUnit->isPcInRange(pc)) {
    //         return compileUnit;
    //     }
    // }
    for (size_t i = compileUnits.size() - 1; i >= 0; i--) {
        if (compileUnits[i]->isPcInRange(pc)) {
            return compileUnits[i];
        }
    }
    return nullptr;
}

size_t DwarfParser::getNumCompileUnits() {
    return compileUnits.size();
}

// Get source information from all compile units
std::vector<SourceInfo *> &DwarfParser::getSourceInfo() {
    if (sourceInfo.empty()) {
        for (CompileUnit *cu : compileUnits) {
            for (SourceInfo *source : cu->getSourceInfo()) {
                if (!LineNumberInfo::containsPath(sourceInfo,
                                                  source->getPath())) {
                    sourceInfo.push_back(source);
                }
            }
        }
    }
    return sourceInfo;
}

uint DwarfParser::getLineNumberAtPc(uint32_t pc) {
    return getCompileUnitAtPc(pc)->getLineNumberAtPc(pc);
}

std::string &DwarfParser::getSourceNameAtPc(uint32_t pc) {
    return getCompileUnitAtPc(pc)->getSourceNameAtPc(pc);
}

void DwarfParser::getLocalVariables(std::vector<Variable *> &variables,
                                    uint32_t pc, uint line) {
    getCompileUnitAtPc(pc)->getLocalVariables(variables, pc, line);
}

void DwarfParser::getGlobalVariables(std::vector<Variable *> &variables,
                                     uint32_t pc, uint line) {
    getCompileUnitAtPc(pc)->getGlobalVariables(variables, pc, line);
}

void DwarfParser::getVarInfo(VarInfo &res, bool doUpdate, Variable *var,
                             RegisterFile *regs, uint32_t pc) {
    var->getVarInfo(res, doUpdate, regs, debugFrame, pc);
}

uint8_t *DwarfParser::getDebugAbbrevStart(void) { return debugAbbrevStart; }

uint8_t *DwarfParser::getDebugFrameStart(void) { return debugFrameStart; }

uint8_t *DwarfParser::getDebugInfoStart(void) { return debugInfoStart; }

uint8_t *DwarfParser::getDebugLineStart(void) { return debugLineStart; }

uint8_t *DwarfParser::getDebugLineStrStart(void) { return debugLineStrStart; }

uint8_t *DwarfParser::getDebugStrStart(void) { return debugStrStart; }

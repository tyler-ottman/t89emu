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

    uint8_t *debugInfoStart = fileStart + debugInfoHeader->offset;
    uint8_t *debugInfoEnd = debugInfoStart + debugInfoHeader->size;
    uint8_t *debugAbbrevStart = fileStart + debugAbbrevHeader->offset;
    uint8_t *debugStrStart = fileStart + debugStrHeader->offset;
    uint8_t *debugLineStrStart = fileStart + debugLineStrHeader->offset;
    uint8_t *debugLineStart = fileStart + debugLineHeader->offset;
    uint8_t *debugFrameStart = fileStart + debugFrameHeader->offset;
    uint8_t *debugFrameEnd = debugFrameStart + debugFrameHeader->size;    

    // Initialize Compile Units and Debug Information Tree
    for (;;) {
        // End of .debug_info section, CU parsing complete
        if ((uintptr_t)debugInfoStart >= ((uintptr_t)debugInfoEnd)) {
            break;
        }

        CompileUnit *compileUnit = new CompileUnit(debugInfoStart,
            debugAbbrevStart, debugStrStart, debugLineStrStart, debugLineStart);
        compileUnits.push_back(compileUnit);
        
        compileUnit->generateScopes();
        // compileUnit->printScopes();

        // Point to next .debug_info CU Header
        debugInfoStart += compileUnit->getDebugInfoLength();

        // Point to next .debug_line CU header
        debugLineStart += compileUnit->getDebugLineLength();
    }

    // Initialize Call Frame Information
    debugFrame = new CallFrameInfo(debugFrameStart, debugFrameEnd);

    // Generate types
}

DwarfParser::~DwarfParser() {
    for (CompileUnit *compileUnit : compileUnits) {
        delete compileUnit;
    }
    compileUnits.clear();
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

void DwarfParser::getVarInfo(Variable::VarInfo &res, bool doUpdate,
                             Variable *var, RegisterFile *regs, uint32_t pc) {
    var->getVarInfo(res, doUpdate, regs, debugFrame, pc);
}
#include "DwarfParser.h"

DwarfParser::DwarfParser(const char *fileName) : ElfParser::ElfParser(fileName) {
    const ElfSectionHeader *debugInfoHeader = getSectionHeader(".debug_info");
    ASSERT(debugInfoHeader, ".debug_info not found\n");
    debugInfo = elfFileInfo->elfData + debugInfoHeader->offset;

    const ElfSectionHeader *debugAbbrevHeader = getSectionHeader(".debug_abbrev");
    ASSERT(debugAbbrevHeader, ".debug_abbrev not found\n");
    debugAbbrev = elfFileInfo->elfData + debugAbbrevHeader->offset;

    
}

DwarfParser::~DwarfParser() {

}
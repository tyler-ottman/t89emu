#include <fstream>

#include "Dwarf/LineNumberInfo.h"

SourceInfo::SourceInfo(std::string &path) : path(path) {
    size_t found = path.find_last_of("/");  // Source Name
    name = path.substr(++found);

    // Store bytes of source file
    std::ifstream fs;
    fs.open(path, std::ifstream::in);
    if (!fs.is_open()) {
        printf("Could not open file: %s\n", name.c_str());
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(fs, line)) {
        lines.push_back(line);
    }

    fs.close();
}

std::string &SourceInfo::getPath() { return path; }

std::string &SourceInfo::getName() { return name; }

std::vector<std::string> &SourceInfo::getLines() { return lines; }

LineNumberInfo::LineNumberInfo(CompileUnit *compileUnit,
        uint8_t *debugLineStart) : compileUnit(compileUnit) {
    stream = new DataStream(debugLineStart, 4 + *((uint32_t *)debugLineStart));

    parseProgramHeader();
    generateFilePaths();

    // Process line number information with state machine
    clearRegisters();
    generateLineNumberMatrix();
}

LineNumberInfo::~LineNumberInfo() {}

// stream should point to directoryEntryFormatCount or fileNameEntryFormatCount
void LineNumberInfo::generateFileFormatInfo(uint8_t &numEntries,
    std::vector<std::pair<ContentEncoding, FormEncoding>> &formatEntries) {
    numEntries = stream->decodeUInt8();
    for (size_t i = 0; i < numEntries; i++) {
        ContentEncoding dirContent = (ContentEncoding)stream->decodeULeb128();
        FormEncoding dirForm = (FormEncoding)stream->decodeULeb128();
        formatEntries.push_back({dirContent, dirForm});
    }
}

// stream should point to directoriesCount or fileNamesCount
void LineNumberInfo::generateFileInformation(uint &numEntries,
    std::vector<std::map<ContentEncoding, DebugData *>> &fileEntries,
    std::vector<std::pair<ContentEncoding, FormEncoding>> &fileFormat) {
    numEntries = stream->decodeULeb128();
    for (size_t i = 0; i < numEntries; i++) {
        std::map<ContentEncoding, DebugData *> dirEntry;
        for (std::pair<ContentEncoding, FormEncoding> &dirFormat : fileFormat) {
            AttributeEntry dirAttr(AtReserved0, dirFormat.second, 0);
            DebugData *dirData = compileUnit->decodeInfo(&dirAttr, stream);
            dirEntry[dirFormat.first] = dirData;
        }
        fileEntries.push_back(dirEntry);
    }
}

// Includes unitLength (4 bytes) field itself
size_t LineNumberInfo::getLength() { return 4 + infoHeader.unitLength; }

std::vector<SourceInfo *> &LineNumberInfo::getSourceInfo() {
    return sourceInfo;
}

uint LineNumberInfo::getLineNumberAtPc(uint32_t pc) {
    LineEntry *lineEntry = getLineEntryAtPc(pc);
    return lineEntry ? lineEntry->line : 0;
}

std::string temp = "";
std::string &LineNumberInfo::getSourceNameAtPc(uint32_t pc) {
    LineEntry *lineEntry = getLineEntryAtPc(pc);
    return lineEntry ? sourceInfo[lineEntry->file]->getName() : temp;
}

bool LineNumberInfo::containsPath(std::vector<SourceInfo *> &sources,
                         std::string &path) {
    for (SourceInfo *source : sources) {
        if (path == source->getPath()) {
            return true;
        }
    }
    return false;
}

void LineNumberInfo::clearRegisters() {
    basicBlock = endSequence = prologueEnd = epilogueBegin = false;
    address = opIndex = column = isa = discriminator = 0;
    isStmt = infoHeader.defaultIsStmt;
    file = line = 1;
}

void LineNumberInfo::generateFilePaths() {
    std::string compileDirectory =
        infoHeader.directories[0][DW_LNCT_path]->getString();
    
    // Generate full paths to directories
    std::vector<std::string> dirPaths;
    for (std::map<ContentEncoding, DebugData *> &directory :
         infoHeader.directories) {
        std::string dirPath = directory[DW_LNCT_path]->getString();
        if (dirPath[0] != '/') { // Make full path
            dirPath = compileDirectory + "/" + dirPath;
        }
        dirPaths.push_back(dirPath);
    }

    // Generate full paths to files
    std::vector<std::string> filePaths;
    for (std::map<ContentEncoding, DebugData *> &file : infoHeader.fileNames) {
        uint directoryIndex = file[DW_LNCT_directory_index]->getUInt();
        std::string filePath = dirPaths[directoryIndex] + "/"
            + file[DW_LNCT_path]->getString();
        sourceInfo.push_back(new SourceInfo(filePath));
    }
}

void LineNumberInfo::generateLineNumberMatrix() {
    // stream->setOffset(12 + infoHeader.headerLength);

    while (stream->isStreamable()) {
        uint8_t opcode = stream->decodeUInt8();
        uint adjustedOpcode;
        uint operationAdvance;

        if (opcode >= infoHeader.opcodeBase) { // Special
            adjustedOpcode = opcode - infoHeader.opcodeBase;
            operationAdvance = adjustedOpcode / infoHeader.lineRange;

            address = getNewAddress(operationAdvance);
            opIndex = getNewOpIndex(operationAdvance);
            line += infoHeader.lineBase + (adjustedOpcode % infoHeader.lineRange);
            lineMatrix.push_back(new LineEntry(address, line, column, file,
                                               isStmt, prologueEnd,
                                               epilogueBegin));
            basicBlock = prologueEnd = epilogueBegin = false;
            discriminator = 0;
        } else if (opcode && opcode < infoHeader.opcodeBase) { // Standard
            switch (opcode) {
            case DW_LNS_copy:
                lineMatrix.push_back(new LineEntry(address, line, column, file,
                                                   isStmt, prologueEnd,
                                                   epilogueBegin));
                discriminator = 0;
                basicBlock = prologueEnd = epilogueBegin = false;
                break;

            case DW_LNS_advance_pc:
                operationAdvance = stream->decodeULeb128();
                address = getNewAddress(operationAdvance);
                opIndex = getNewOpIndex(operationAdvance);
                break;

            case DW_LNS_advance_line: line += stream->decodeLeb128(); break;

            case DW_LNS_set_file: file = stream->decodeULeb128(); break;

            case DW_LNS_set_column: column = stream->decodeULeb128(); break;

            case DW_LNS_negate_stmt: isStmt = !isStmt;  break;

            case DW_LNS_set_basic_block: basicBlock = true; break;

            case DW_LNS_const_add_pc:
                adjustedOpcode = 255 - infoHeader.opcodeBase;
                operationAdvance = adjustedOpcode / infoHeader.lineRange;
                address = getNewAddress(operationAdvance);
                opIndex = getNewOpIndex(operationAdvance);
                break;

            case DW_LNS_fixed_advance_pc:
                address += stream->decodeUInt16();
                opIndex = 0;
                break;

            case DW_LNS_set_prologue_end: prologueEnd = true; break;

            case DW_LNS_set_epilogue_end: epilogueBegin = true; break;

            case DW_LNS_set_isa: isa = stream->decodeULeb128(); break;
            }
        } else { // Extended
            stream->decodeULeb128(); // instruction length
            opcode = stream->decodeUInt8();

            switch (opcode) {
            case DW_LNE_end_sequence:
                endSequence = true;
                lineMatrix.push_back(new LineEntry(address, line, column, file,
                                                   isStmt, prologueEnd,
                                                   epilogueBegin));
                clearRegisters();
                break;

            case DW_LNE_set_address:
                address = stream->decodeUInt32();
                opIndex = 0;
                break;

            case DW_LNE_set_discriminator:
                discriminator = stream->decodeULeb128();
                break;
            }
        }
    }
}

void LineNumberInfo::parseProgramHeader() {
    infoHeader = {
        .unitLength = stream->decodeUInt32(),
        .version = stream->decodeUInt16(),
        .addressSize = stream->decodeUInt8(),
        .segmentSelectorSize = stream->decodeUInt8(),
        .headerLength = stream->decodeUInt32(),
        .minimumInstructionLength = stream->decodeUInt8(),
        .maximumOperationsPerInstruction = stream->decodeUInt8(),
        .defaultIsStmt = stream->decodeUInt8(),
        .lineBase = (int8_t)stream->decodeUInt8(),
        .lineRange = stream->decodeUInt8(),
        .opcodeBase = stream->decodeUInt8()
    };

    for (int i = 0; i < infoHeader.opcodeBase - 1; i++) {
        infoHeader.standardOpcodeLengths.push_back(stream->decodeUInt8());
    }

    // Directory Information
    generateFileFormatInfo(infoHeader.directoryEntryFormatCount,
                           infoHeader.directoryEntryFormat);
    generateFileInformation(infoHeader.directoriesCount, infoHeader.directories,
                            infoHeader.directoryEntryFormat);

    // File Information
    generateFileFormatInfo(infoHeader.fileNameEntryFormatCount,
                           infoHeader.fileNameEntryFormat);
    generateFileInformation(infoHeader.fileNamesCount, infoHeader.fileNames,
                            infoHeader.fileNameEntryFormat);
}

uint LineNumberInfo::getNewAddress(uint operationAdvance) {
    return address + infoHeader.minimumInstructionLength *
                         ((opIndex + operationAdvance) /
                          infoHeader.maximumOperationsPerInstruction);
}

uint LineNumberInfo::getNewOpIndex(uint operationAdvance) {
    return (opIndex + operationAdvance) %
           infoHeader.maximumOperationsPerInstruction;
}

LineNumberInfo::LineEntry *LineNumberInfo::getLineEntryAtPc(uint32_t pc) {
    for (LineEntry *lineEntry : lineMatrix) {
        if (lineEntry->address == pc) {
            return lineEntry;
        }
    }
    return nullptr;
}
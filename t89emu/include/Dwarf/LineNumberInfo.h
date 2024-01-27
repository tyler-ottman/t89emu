#pragma once

#include <map>
#include <utility>
#include <vector>

#include "Dwarf/CompileUnit.h"
#include <Dwarf/DebugInfoEntry.h>
#include "Dwarf/DwarfEncodings.h"

class CompileUnit;
class DataStream;
class DebugData;

class SourceInfo {
public:
    SourceInfo(std::string &path);
    ~SourceInfo();

    std::string &getPath(void);
    std::string &getName(void);
    std::vector<std::string> &getLines(void);

private:
    std::string path;
    std::string name;
    std::vector<std::string> lines;
};

class LineNumberInfo {
public:
    LineNumberInfo(CompileUnit *cu, uint8_t *debugLineStart);
    ~LineNumberInfo();

    void generateFileFormatInfo(uint8_t &numEntries,
        std::vector<std::pair<ContentEncoding, FormEncoding>> &formatEntries);
    void generateFileInformation(uint &numEntries,
        std::vector<std::map<ContentEncoding, DebugData *>> &fileEntries,
        std::vector<std::pair<ContentEncoding, FormEncoding>> &fileFormat);

    // get total length of the line number information for this compile unit
    size_t getLength(void);
    std::vector<SourceInfo *> &getSourceInfo(void);
    uint getLineNumberAtPc(uint32_t pc);
    std::string &getSourceNameAtPc(uint32_t pc);

    static bool containsPath(std::vector<SourceInfo *> &sources,
                             std::string &path);

private:
    struct ProgramHeader {
        uint32_t unitLength;
        uint16_t version;
        uint8_t addressSize;
        uint8_t segmentSelectorSize;
        uint32_t headerLength;
        uint8_t minimumInstructionLength;
        uint8_t maximumOperationsPerInstruction;
        uint8_t defaultIsStmt;
        int8_t lineBase;
        uint8_t lineRange;
        uint8_t opcodeBase;
        std::vector<uint8_t> standardOpcodeLengths;
        uint8_t directoryEntryFormatCount;
        std::vector<std::pair<ContentEncoding, FormEncoding>>
            directoryEntryFormat;
        uint directoriesCount;
        std::vector<std::map<ContentEncoding, DebugData *>> directories;
        uint8_t fileNameEntryFormatCount;
        std::vector<std::pair<ContentEncoding, FormEncoding>>
            fileNameEntryFormat;
        uint fileNamesCount;
        std::vector<std::map<ContentEncoding, DebugData *>> fileNames;
    };

    struct LineEntry {
        LineEntry(uint32_t address, uint line, uint column, uint file,
                  bool isStmt, bool prologueEnd, bool epilogueBegin)
            : address(address), line(line), column(column), file(file),
              isStmt(isStmt), prolgueEnd(prologueEnd),
              epilogueBegin(epilogueBegin) {}
        uint32_t address;
        uint line;
        uint column;
        uint file;
        bool isStmt; // flags for setting breakpoints
        bool prolgueEnd;
        bool epilogueBegin;
    };

    void clearRegisters(void); // End of every sequence
    void generateFilePaths(void);
    void generateLineNumberMatrix(void);
    void parseProgramHeader(void);

    uint getNewAddress(uint operationAdvance);
    uint getNewOpIndex(uint opeartionAdvance);
    LineEntry *getLineEntryAtPc(uint32_t pc);

    ProgramHeader infoHeader;
    std::vector<LineEntry *> lineMatrix;
    std::vector<SourceInfo *> sourceInfo;
    DataStream *stream; // .debug_line stream
    CompileUnit *compileUnit;

    uint32_t address;
    uint opIndex;
    uint file;
    uint line;
    uint column;
    bool isStmt;
    bool basicBlock;
    bool endSequence;
    bool prologueEnd;
    bool epilogueBegin;
    uint isa;
    uint discriminator;
};
#pragma once

#include <unordered_map>

#include "Dwarf/DataStream.h"
#include "Dwarf/DwarfEncodings.h"
#include "RegisterFile.h"

class CallFrameInfo {
public:
    CallFrameInfo(uint8_t *debugFrameStart, uint8_t *debugFrameEnd);
    ~CallFrameInfo();

    bool getCfaAtLocation(uint32_t &res, uint32_t l1, RegisterFile *regs);
    bool getRegAtLocation(uint32_t &res, uint column, uint32_t l1,
                          RegisterFile *regs);

private:
    struct CommonInfoEntry {
        size_t getLen(void) { return sizeof(length) + length; }
        uint32_t length;
        uint32_t cieId;
        uint8_t version;
        std::vector<char> augmentation;
        uint8_t addressSize;
        uint8_t segmentSelectorSize;
        uint64_t codeAlignmentFactor;
        int64_t dataAlignmentFactor;
        uint64_t returnAddressRegister;
        const uint8_t *initialInstructions;
        size_t instructionsBlockSize;
    };

    struct FrameDescriptEntry {
        size_t getLen(void) { return sizeof(length) + length; }
        uint32_t length;
        uint32_t ciePointer;
        uint32_t initialLocation;
        uint32_t addressRange;
        const uint8_t *instructions;
        size_t instructionsBlockSize;
    };

    enum CfiRule {
        cfiUndefined,
        cfiSameValue,
        cfiOffset,
        cfiValOffset,
        cfiRegister,
        cfiExpression,
        cfiValExpression,
        cfiArchitectural
    };

    struct RegRule {
        uint32_t value;
        CfiRule cfiRule;
    };

    struct TableEntry {
        bool isCfaLocExpr = false;
        uint32_t regNum;
        union {
            uint32_t regOffset;
            uint32_t location;
        };

        // Register number -> Register rule
        std::unordered_map<uint, RegRule> regRules;
    };

    bool generateVirtTable(std::vector<TableEntry> &virtTable, uint32_t l1,
                           RegisterFile *regs);
    void generateFrameInfo(void);
    CommonInfoEntry *parseCie(uint32_t length, uint32_t cieId,
                              uint32_t debugFrameOffset);
    FrameDescriptEntry *parseFde(uint32_t length, uint32_t ciePointer);
    void processInstructions(std::vector<TableEntry> &virtTable,
                             CommonInfoEntry *cie, DataStream &instrStream,
                             RegisterFile *regs, uint32_t l1, uint32_t l2);
    bool getUnwindedValue(uint32_t &res, std::vector<TableEntry> &virtTable,
                          uint regCol, RegisterFile *regs, int depth);

    std::unordered_map<size_t, CommonInfoEntry *> cies;
    std::vector<FrameDescriptEntry *> fdes;

    DataStream *cfiStream;
};
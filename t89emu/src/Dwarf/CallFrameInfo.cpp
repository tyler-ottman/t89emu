#include "Dwarf/CallFrameInfo.h"

CallFrameInfo::CallFrameInfo(uint8_t *debugFrameStart, uint8_t *debugFrameEnd) {
    size_t debugFrameLen = (uintptr_t)(debugFrameEnd - debugFrameStart);
    cfiStream = new DataStream(debugFrameStart, debugFrameLen);
    generateFrameInfo();
}

CallFrameInfo::~CallFrameInfo() {}

bool CallFrameInfo::getCfaAtLocation(uint32_t &res, uint32_t l1,
                                     RegisterFile *regs) {
    std::vector<TableEntry> virtualTable(1);
    if (!generateVirtTable(virtualTable, l1, regs)) { return false; }

    TableEntry &entry = virtualTable.back();
    res = regs->read(entry.regNum) + entry.regOffset;
    return true;
}

bool CallFrameInfo::getRegAtLocation(uint32_t &res, uint column, uint32_t l1,
                                     RegisterFile *regs) {
    std::vector<TableEntry> virtualTable(1);
    if (!generateVirtTable(virtualTable, l1, regs)) { return false; }

    return getUnwindedValue(res, virtualTable, column, regs,
                            virtualTable.size() - 1);
}

bool CallFrameInfo::generateVirtTable(std::vector<TableEntry> &virtTable,
                                      uint32_t l1, RegisterFile *regs) {
    // Find the FDE contaning addr
    FrameDescriptEntry *fde = nullptr;
    for (FrameDescriptEntry *entry : fdes) {
        uint32_t baseAddr = entry->initialLocation;
        if (l1 >= baseAddr && l1 < baseAddr + entry->addressRange) {
            fde = entry;
        }
    }
    if (fde == nullptr || cies.find(fde->ciePointer) == cies.end()) {
        return false;
    }

    // 1. Initialize Register Set (Virtual Table) and read CIE's initial
    // instructions, then set L2 to initialLocation
    CommonInfoEntry *cie = cies[fde->ciePointer];
    DataStream cieStream(cie->initialInstructions, cie->instructionsBlockSize);
    uint32_t l2 = fde->initialLocation;
    for (uint i = 0; i < 32; i++) {
        virtTable.back().regRules[i] = {.cfiRule = cfiUndefined};
    }
    virtTable[0].location = fde->initialLocation;
    processInstructions(virtTable, cie, cieStream, regs, l1, l2);

    // 2. Process FDE instruction stream
    DataStream fdeStream(fde->instructions, fde->instructionsBlockSize);
    processInstructions(virtTable, cie, fdeStream, regs, l1, l2);

    return true;
}

bool CallFrameInfo::getUnwindedValue(uint32_t &res,
                                     std::vector<TableEntry> &virtTable,
                                     uint regCol, RegisterFile *regs,
                                     int depth) {
    return false;
}

void CallFrameInfo::generateFrameInfo() {
    while (cfiStream->isStreamable()) {
        size_t entryLen;
        size_t debugFrameOffset = cfiStream->getIndex();
        uint32_t length = cfiStream->decodeUInt32();
        uint32_t identifier = cfiStream->decodeUInt32();

        if (identifier == 0xffffffff) {
            // 2nd param specifies CIE's byte offset into .debug_frame
            CommonInfoEntry *cie = parseCie(length, identifier,
                                            cfiStream->getIndex() - 8);
            entryLen = cie->getLen();
        } else {
            FrameDescriptEntry *fde = parseFde(length, identifier);
            entryLen = fde->getLen();
        }

        cfiStream->setOffset(debugFrameOffset + entryLen);
    }
}

CallFrameInfo::CommonInfoEntry *CallFrameInfo::parseCie(
    uint32_t length, uint32_t cieId, uint32_t debugFrameOffset) {
    CommonInfoEntry *cie = new CommonInfoEntry();
    cie->length = length;
    cie->cieId = cieId;
    cie->version = cfiStream->decodeUInt8();
    
    char c;
    while ((c = cfiStream->decodeUInt8())) {
        cie->augmentation.push_back(c);
    }
    
    // cie->addressSize = cfiStream->decodeUInt8();
    // cie->segmentSelectorSize = cfiStream->decodeUInt8();
    cie->codeAlignmentFactor = cfiStream->decodeULeb128();
    cie->dataAlignmentFactor = cfiStream->decodeLeb128();
    cie->returnAddressRegister = cfiStream->decodeULeb128();
    cie->initialInstructions = cfiStream->getData(cfiStream->getIndex());
    cie->instructionsBlockSize =
        cie->getLen() - (cfiStream->getIndex() - debugFrameOffset);
    cies.insert({debugFrameOffset, cie});

    return cie;
}

CallFrameInfo::FrameDescriptEntry *CallFrameInfo::parseFde(
    uint32_t length, uint32_t ciePointer) {
    uint startIdx = cfiStream->getIndex() - 8; // First index into FDE
    FrameDescriptEntry *fde = new FrameDescriptEntry();
    fde->length = length;
    fde->ciePointer = ciePointer;
    fde->initialLocation = cfiStream->decodeUInt32();
    fde->addressRange = cfiStream->decodeUInt32();
    fde->instructions = cfiStream->getData(cfiStream->getIndex());
    fde->instructionsBlockSize =
        fde->getLen() - (cfiStream->getIndex() - startIdx);
    fdes.push_back(fde);

    return fde;
}

void CallFrameInfo::processInstructions(std::vector<TableEntry> &virtTable,
                             CommonInfoEntry *cie, DataStream &instrStream,
                             RegisterFile *regs, uint32_t l1, uint32_t l2) {
    while (instrStream.isStreamable()) {
        uint8_t opcode = instrStream.decodeUInt8();

        uint8_t primaryOpcode = opcode & 0xc0;
        if (primaryOpcode) {
            uint8_t operand = opcode & 0x4f;

            switch (primaryOpcode) {
            case DW_CFA_advance_loc: {
                TableEntry entry(virtTable.back());
                entry.location = virtTable.back().location +
                                 operand * cie->codeAlignmentFactor;
                virtTable.push_back(entry);
                break;
            }

            case DW_CFA_offset:
                virtTable.back().regRules[operand].value =
                    instrStream.decodeULeb128() * cie->dataAlignmentFactor;
                break;

            case DW_CFA_restore:
                virtTable.back().regRules[operand].cfiRule =
                    virtTable.front().regRules[operand].cfiRule;
                break;

            default:
                printf("Illegal CFA: %x\n", primaryOpcode); exit(1);
            }
            continue;
        }

        switch (opcode) {
        case DW_CFA_nop: break;
        
        case DW_CFA_advance_loc1: {
            uint32_t delta = instrStream.decodeUInt8();
            TableEntry entry(virtTable.back());
            entry.location = virtTable.back().location + delta;
            virtTable.push_back(entry);
            break;
        }

        case DW_CFA_def_cfa: {
            virtTable.back().regNum = instrStream.decodeULeb128();
            virtTable.back().regOffset = instrStream.decodeULeb128();
            break;
        }

        case DW_CFA_def_cfa_register:
            if (virtTable.back().isCfaLocExpr) {
                printf("%s error\n", printCfa((CfaEncoding)opcode)); exit(1);
            }
            virtTable.back().regNum = instrStream.decodeULeb128();
            break;

        case DW_CFA_def_cfa_offset:
            if (virtTable.back().isCfaLocExpr) {
                printf("%s error\n", printCfa((CfaEncoding)opcode)); exit(1);
            }
            virtTable.back().regOffset = instrStream.decodeULeb128();
            break;

        default:
            printf("Unsupported CFA: %s\n",
                   printCfa((CfaEncoding)opcode));
            exit(EXIT_FAILURE);
        }
    }
}
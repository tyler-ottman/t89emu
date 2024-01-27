#include "Dwarf/DwarfExpr.h"

StackMachine::StackMachine() {
    exprStream = new DataStream(nullptr);
}

StackMachine::~StackMachine() {}

DebugData StackMachine::parseExpr(DebugData *expr, RegisterFile *regs,
                                  CallFrameInfo *cfi, uint32_t pc) {
    return parseExpr(expr, regs, cfi, pc, nullptr);
}

DebugData StackMachine::parseExpr(DebugData *expr, RegisterFile *regs,
                                  CallFrameInfo *cfi, uint32_t pc,
                                  Variable *var) {
    DebugData res;
    exprStream->setData(expr->getData());
    exprStream->setLen(expr->getLen());

    while (exprStream->isStreamable()) {
        OperationEncoding opcode = (OperationEncoding)exprStream->decodeUInt8();

        switch (opcode) {
        case DW_OP_lit0 ... DW_OP_lit31:
            stack.push(opcode - DW_OP_lit0);
            break;

        case DW_OP_addr:
            stack.push(exprStream->decodeUInt32());
            break;

        case DW_OP_fbreg: {
            if (var == nullptr) {
                printf("Unexpected DW_OP_fbreg"); exit(1);
            }
            int32_t offset = exprStream->decodeLeb128();

            // DW_AT_frame_base is DWARF location expression
            DebugInfoEntry *parent = var->getParentEntry();
            DebugData *fbLoc = parent->getAttribute(DW_AT_frame_base);
            StackMachine fbStack;
            DebugData location = fbStack.parseExpr(fbLoc, regs, cfi, pc);
            stack.push(offset + location.getUInt());
            break;
        }

        case DW_OP_call_frame_cfa: {
            uint32_t cfa;
            bool ret = cfi->getCfaAtLocation(cfa, pc, regs);
            if (!ret) { printf("%s error\n", printOperation(opcode)); exit(1);}
            stack.push(cfa);
            break;
        }
            
        default:
            printf("StackMachine: Unknown OP: %s\n",
                   printOperation((OperationEncoding)opcode));
            exit(1);
        }
    }

    res.write((uint8_t *)&stack.top(), sizeof(GenericType));
    return res;
}
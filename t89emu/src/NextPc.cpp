#include <iostream>

#include "NextPc.h"

NextPc::NextPc() {
    this->nextPc = 0;
}

int NextPc::branchAlu(uint32_t A, uint32_t B, uint32_t funct3) {
    uint32_t ALU_out =  A - B; // Same hardware as main ALU
    switch (funct3) {
        case 0b000: return !ALU_out; // beq
        case 0b001: return ALU_out; // bne
        case 0b100: return (ALU_out >> 31) == 1; // blt
        case 0b101: return (ALU_out >> 31) == 0; // bge
        case 0b110: return (A < B); // bltu
        case 0b111: return (A >= B); // bgeu
    }
    return 0;
}

uint32_t NextPc::calculateNextPc(uint32_t offset, uint32_t opcode, uint32_t funct3, uint32_t A, uint32_t B, uint32_t mepc) {
    switch (opcode) {
        case JAL: nextPc += offset; break; // JAL signal
        case JALR: nextPc = A + offset; break; // JALR signal
        case BTYPE: nextPc += branchAlu(A, B, funct3) ? offset : 4; break; // B-type signal
        case PRIV:
            if ((funct3 == 0) && (offset == MRET_IMM)) { // MRET
                nextPc = mepc;
            } else { // CSR Instruction
                nextPc += 4;
            }
            break;
        default: nextPc += 4; break; // All other instructions
    }
    if (nextPc % 4 != 0) {
        return INSTRUCTION_ADDRESS_MISALIGNED;
    }

    return STATUS_OK;
}
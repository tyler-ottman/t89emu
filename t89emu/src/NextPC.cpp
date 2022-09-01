#include <iostream>
#include "Components.h"

NextPC::NextPC() {
    this->nextPC = 0;
}

int NextPC::branch_alu(uint32_t A, uint32_t B, uint32_t funct3) {
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

uint32_t NextPC::calculateNextPC(uint32_t offset, uint32_t opcode, uint32_t funct3, uint32_t A, uint32_t B, uint32_t mtvec, uint32_t interrupt_taken, uint32_t mepc) {
    if (interrupt_taken) {
        // Invoked by exceptions or non instruction interrupts
        this->nextPC = mtvec;
        return this->nextPC;
    }

    switch (opcode) {
        case JAL: this->nextPC += offset; break; // JAL signal
        case JALR: this->nextPC = A + offset; break; // JALR signal
        case BTYPE: this->nextPC += branch_alu(A, B, funct3) ? offset : 4; break; // B-type signal
        case PRIV:
            if ((funct3 == 0) && (offset == MRET_IMM)) { // ECALL/MRET
                switch(offset) {
                case ECALL_IMM: this->nextPC += 4; break; // If here then software interrupts are disabled
                case MRET_IMM:  this->nextPC = mepc; break;
                }
            } else { // CSR Instruction
                this->nextPC += 4;
            }
            break;
        default: this->nextPC += 4; break; // All other instructions
    }
    return this->nextPC;
}
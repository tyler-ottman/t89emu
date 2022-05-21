#include <iostream>
#include <stdlib.h>
#include "Components.h"

ControlUnit::ControlUnit(){
    // Each signal is 2-bit width
    // RegWrite - Enable writing to Register File
    // ALUSrc - Select 4, rs2, or immediate to feed into B of ALU
    // MemtoReg - Select output of ALU or output of memory to send to Register File
    // MemRead - Read data from memory
    // MemWrite - Write data to memory
    // PC_select - Select rs1, PC, or 0 to feed to A of ALU
    signals[LUI]   = 0b011000000010;
    signals[AUIPC] = 0b011000000001;
    signals[JAL]   = 0b010000000001;
    signals[JALR]  = 0b010000000001;
    signals[BTYPE] = 0b000100000000;
    signals[LOAD]  = 0b011001010000;
    signals[STORE] = 0b001000000100;
    signals[ITYPE] = 0b011000000000;
    signals[RTYPE] = 0b010100000000;
    signals[ECALL] = 0b000010000000;
}

std::vector<uint32_t> ControlUnit::getControlLines(int opcode, int interrupt, int funct3, int supervisor_mode)
{
    if (opcode == ECALL && funct3 == 0b000) {
        //      signals,        trap, mcause,     CSR_enable
        return {0b000000000000, 1,    0x00000008, 0};
    } else if (interrupt != 0 && supervisor_mode) { // Hardware/Timer Interrupt (Machine Mode)
        switch(interrupt) {
            // All signals disabled, trap high
            case 1: // Machine Timer Interrupt
                return {0b000000000000, 1, 0x80000007, 0};
            case 2: // Machine External Interrupt
                return {0b000000000000, 1, 0x8000000b, 0};
        }
    }

    if(opcode == ECALL && supervisor_mode && funct3 == 0b001)
        return {signals[ECALL], 0, 0, 1}; // CSRRW
    
    // Normal instructions
    return {signals[opcode], 0, 0, 0};
}

class ControlUnit;
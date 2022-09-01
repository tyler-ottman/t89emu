#include <iostream>
#include <math.h>
#include <vector>
#include "Components.h"

uint32_t ImmediateGenerator::getImmediate(uint32_t instruction)
{
    uint32_t opcode = instruction & 0b1111111;
    uint32_t immediate = 0;
    uint32_t funct3;

    switch (opcode)
    {
    case LUI: // lui
        return instruction & 0xfffff000;
    case AUIPC: // auipc
        return instruction & 0xfffff000;
        break;
    case JAL: // jal
		immediate = ((instruction >> 12) & 0x00080000) | // imm[20]
		            ((instruction >> 1)  & 0x0007f800) | // imm[19:12]
		            ((instruction >> 10) & 0x00000400) | // imm[11]
		            ((instruction >> 21) & 0x000003ff);  // imm[10:1]
		if (((immediate >> 19) & 0b1) == 1) // sign extend if negative offset
			immediate |= 0xfff00000;
		return immediate <<= 1; // Shift immediate left by 1 bit
    case JALR: // jalr
        immediate = (instruction >> 20) & 0xfff; // 12 bit immediate
        if (((immediate >> 11) & 0b1) == 1) // sign extend if negative offset
            immediate |= 0xfffff000;
        return immediate;
    case BTYPE: // B-type
		immediate = ((instruction >> 20) & 0x0000800) | // imm[12]
	                ((instruction << 3)  & 0x0000400) | // imm[11]
			        ((instruction >> 21) & 0x00003f0) | // imm[10:5]
			        ((instruction >> 8)  & 0x000000f);  // imm[4:1]
		if (((immediate >> 11) & 0x1) == 1) // sign extend if negative offset
			immediate |= 0xfffff000;
		return immediate <<= 1; // shift offset left by 1
    case LOAD: // Loads
        immediate = instruction >> 20; // 12 bit immediate
        if (instruction >> 31) {
            // MSB of immediate is 1 (store backwards)
            immediate |= 0xfffff000;
        }
        return immediate;
    case STORE: // Stores
        immediate = ((instruction >> 20) & (0b1111111 << 5)) + ((instruction >> 7) & 0b11111);
        if (instruction >> 31) {
            // MSB of immediate is 1 (store backwards)
            immediate |= 0xfffff000;
        }
        return immediate;
    case ITYPE: // Immediate Arithmetic
        immediate = instruction >> 20;
        if (instruction >> 31) {
            // MSB of immediate is 1 (negative immediate)
            immediate |= 0xfffff000;
        }
        funct3 = (instruction >> 12) & 0b111;
        if (funct3 == 0b001 || funct3 == 0b101) {
            immediate = immediate & 0b11111;        // slli, srli, srai
        }
        return immediate;
    case PRIV: // ecall/csri(w)
        return ((instruction >> 20) & 0xfff);
    }
    return immediate;
}
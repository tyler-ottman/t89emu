#include <iostream>
#include <math.h>
#include <vector>
#include "Components.h"

template <typename T>
T ImmediateGenerator<T>::getInstrType()
{
    for (size_t i = 0; i < this->opcodes.size(); i++)
        if (this->opcodes[i] == this->opcode)
            return i;
    return 0;
}

template <typename T>
ImmediateGenerator<T>::ImmediateGenerator() {
    this->opcodes = {0b0110111, 0b0010111, 0b1101111, 0b1100111, 0b1100011,
                    0b0000011, 0b0100011, 0b0010011, 0b1110011, 0b0110011};
    this->immediate = 0;
}

template <typename T>
T ImmediateGenerator<T>::getImmediate(T instruction)
{
    this->opcode = instruction & 0b1111111;
    switch (getInstrType())
    {
    case 0: // lui
        return instruction & 0xfffff000;
    case 1: // auipc
        return instruction & 0xfffff000;
        break;
    case 2: // jal
		this->immediate = ((instruction >> 12) & 0x00080000) | // imm[20]
		            ((instruction >> 1)  & 0x0007f800) | // imm[19:12]
		            ((instruction >> 10) & 0x00000400) | // imm[11]
		            ((instruction >> 21) & 0x000003ff);  // imm[10:1]
		if (((this->immediate >> 19) & 0b1) == 1) // sign extend if negative offset
			this->immediate |= 0xfff00000;
		return this->immediate <<= 1; // Shift immediate left by 1 bit
    case 3: // jalr
        this->immediate = (instruction >> 20) & 0xfff; // 12 bit immediate
        if (((this->immediate >> 11) & 0b1) == 1) // sign extend if negative offset
            this->immediate |= 0xfffff000;
        return this->immediate;
    case 4: // B-type
		this->immediate = ((instruction >> 20) & 0x0000800) | // imm[12]
	                ((instruction << 3)  & 0x0000400) | // imm[11]
			        ((instruction >> 21) & 0x00003f0) | // imm[10:5]
			        ((instruction >> 8)  & 0x000000f);  // imm[4:1]
		if (((this->immediate >> 11) & 0x1) == 1) // sign extend if negative offset
			this->immediate |= 0xfffff000;
		return this->immediate <<= 1; // shift offset left by 1
    case 5: // Loads
        this->immediate = instruction >> 20; // 12 bit immediate
        if (instruction >> 31) {
            // MSB of immediate is 1 (store backwards)
            this->immediate |= 0xfffff000;
        }
        return this->immediate;
    case 6: // Stores
        this->immediate = ((instruction >> 20) & (0b1111111 << 5)) + ((instruction >> 7) & 0b11111);
        if (instruction >> 31) {
            // MSB of immediate is 1 (store backwards)
            this->immediate |= 0xfffff000;
        }
        return this->immediate;
    case 7: // Immediate Arithmetic
        this->immediate = instruction >> 20;
        if (instruction >> 31) {
            // MSB of immediate is 1 (negative immediate)
            this->immediate |= 0xfffff000;
        }
        this->funct3 = (instruction >> 12) & 0b111;
        if (this->funct3 == 0b001 || this->funct3 == 0b101) {
            this->immediate = this->immediate & 0b11111;        // slli, srli, srai
        }
        return this->immediate;
    case 8: // ecall/csri(w)
        return instruction >> 20;
    }
    return this->immediate;
}

template class ImmediateGenerator<uint32_t>;
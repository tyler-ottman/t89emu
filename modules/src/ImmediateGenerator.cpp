#include <iostream>
#include <math.h>
#include "Components.h"

template <typename T>T instruction;
int instr_opcode;

template <typename T>
int ImmediateGenerator<T>::getInstrType()
{
    int opcodes[9] =    {0b0110111, 0b0010111, 0b1101111, 0b1100111, 0b1100011,
                         0b0000011, 0b0100011, 0b0010011, 0b1110011};
    for (int i = 0; i < 9; i++) {
        if (opcodes[i] == this->instr_opcode) {
            return i;
        }
    }
    if (this->instr_opcode == 0b0110011) {
        return 0;
    }
    std::cerr << "Invalid Opcode Error" << std::endl;
    exit(EXIT_FAILURE);
}

template <typename T>
T* ImmediateGenerator<T>::getBinaryArray(T A)
{
    T *bin = (T *)malloc(32 * sizeof(T));
    for (int i = 0; i < 32; i++)
    {
        bin[i] = ((A >> (32 - i - 1)) & 0x1);
    }
    return bin;
}

template <typename T>
T ImmediateGenerator<T>::getNum(T *arr, int size)
{
    T sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += arr[i] * (T)pow(2, size - 1 - i);
    }
    return sum;
}

template <typename T>
ImmediateGenerator<T>::ImmediateGenerator() {}

template <typename T>
T ImmediateGenerator<T>::getImmediate(T instruction)
{
    this->instr_opcode = instruction & 0b1111111; // opcode: 6 - 0
    int instrType = getInstrType();
    int funct3;
    T immediate = 0;
    switch (instrType)
    {
    case 0: // lui
        immediate = instruction & 0xfffff000;
        break;
    case 1: // auipc
        immediate = instruction & 0xfffff000;
        break;
    case 2: // jal
		immediate = ((instruction >> 12) & 0x00080000) | // imm[20]
		            ((instruction >> 1)  & 0x0007f800) | // imm[19:12]
		            ((instruction >> 10) & 0x00000400) | // imm[11]
		            ((instruction >> 21) & 0x000003ff);  // imm[10:1]
		if (((immediate >> 19) & 0b1) == 1) // sign extend if negative offset
			immediate |= 0xfff00000;
		immediate <<= 1; // Shift immediate left by 1 bit
        break;
    case 3: // jalr
        immediate = (instruction >> 20) & 0xfff; // 12 bit immediate
        if (((immediate >> 11) & 0b1) == 1) // sign extend if negative offset
            immediate |= 0xfffff000;
        break;
    case 4: // B-type
		immediate = ((instruction >> 20) & 0x0000800) | // imm[12]
	                ((instruction << 3)  & 0x0000400) | // imm[11]
			        ((instruction >> 21) & 0x00003f0) | // imm[10:5]
			        ((instruction >> 8)  & 0x000000f);  // imm[4:1]
		if (((immediate >> 11) & 0x1) == 1) // sign extend if negative offset
			immediate |= 0xfffff000;
		immediate <<= 1; // shift offset left by 1
        break;
    case 5: // Loads
        immediate = instruction >> 20; // 12 bit immediate
        if (instruction >> 31) {
            // MSB of immediate is 1 (store backwards)
            immediate |= 0xfffff000;
        }
        break;
    case 6: // Stores
        immediate = ((instruction >> 20) & (0b1111111 << 5)) + ((instruction >> 7) & 0b11111);
        if (instruction >> 31) {
            // MSB of immediate is 1 (store backwards)
            immediate |= 0xfffff000;
        }
        break;
    case 7: // Immediate Arithmetic
        immediate = instruction >> 20;
        if (instruction >> 31) {
            // MSB of immediate is 1 (negative immediate)
            immediate |= 0xfffff000;
        }
        funct3 = (instruction >> 12) & 0b111;
        if (funct3 == 0b001 || funct3 == 0b101) {
            immediate = immediate & 0b11111;        // slli, srli, srai
        }
        break;
    case 8: // ecall/csri(w)
        immediate = instruction >> 20;
        break;
    }
    return immediate;
}

template class ImmediateGenerator<uint32_t>;
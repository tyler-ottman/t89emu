#include <iostream>
#include "Components.h"

template <typename T>T nextPC;

template <typename T>
int NextPC<T>::jal_jalr_branch(T opcode)
{
    //              jal,       jalr,      B-type,    ecall/csr
    T opcodes[4] = {0b1101111, 0b1100111, 0b1100011, 0b1110011};
    for (int i = 0; i < 4; i++) {
        if (opcode == opcodes[i]) {
            return i;
        }
    }
    // All other types of instructions
    return 4;
}

template <typename T>
int NextPC<T>::branch_alu(T A, T B, T funct3)
{
    T ALU_in =  A - B; // Same hardware as main ALU
    switch (funct3)
    {
    case 0b000: // beq
        if (!ALU_in)
        {
            return 1;
        }
        return 0;
    case 0b001: // bne
        if (ALU_in)
        {
            return 1;
        }
        return 0;
    case 0b100: // blt
        if ((ALU_in >> 31))
        {
            // MSB is 1 --> negative
            return 1;
        }
        return 0;
    case 0b101: // bge
        return (ALU_in >> 31) == 0;
    case 0b110: // bltu
        // If a and b are positive
        if (((A >> 31) == 0) && ((B >> 31) == 0)) {
            return (ALU_in >> 31) == 1;
        } else if (((A >> 31) == 1) && ((B >> 31) == 1)) {
            // a negative, b negative
            return (ALU_in >> 31) == 1;
        } else if (((A >> 31) == 0) && ((B >> 31) == 1)) {
            // a positive, b "negative"
            return 1;
        } else {
            // a "negative", b positive
            return 0;
        }

    case 0b111: // bgeu
        // return (ALU_in >> 31) == 0;
        // If a and b are positive
        if (((A >> 31) == 0) && ((B >> 31) == 0)) {
            return (ALU_in >> 31) == 0;
        } else if (((A >> 31) == 1) && ((B >> 31) == 1)) {
            // a negative, b negative
            return (ALU_in >> 31) == 0;
        } else if (((A >> 31) == 0) && ((B >> 31) == 1)) {
            // a positive, b "negative"
            return 0;
        } else {
            // a "negative", b positive
            return 1;
        }
    }
    return 0;
}

template <typename T>
NextPC<T>::NextPC(){
    this->nextPC = 0;
}

template <typename T>
void NextPC<T>::setCurrentPC(T currentPC) {
    this->nextPC = currentPC;
}

template <typename T>
void NextPC<T>::calculateNextPC(T offset, T opcode, T funct3, T A, T B, T mtvec, T interrupt_taken)
{
    int instr_type = jal_jalr_branch(opcode);
    int branch = 0;
    switch (instr_type)
    {
    case 0: // jal signal
        this->nextPC += offset;
        break;
    case 1: // jalr signal
        // this->nextPC = A + (offset << 2);
        this->nextPC = A + offset;
        break;
    case 2: // B-type signal
        branch = branch_alu(A, B, funct3);
        if (branch)
        {
            // this->nextPC += (offset << 2);
            this->nextPC += offset;
        }
        else
        {
            // no branch
            this->nextPC += 4;
        }
        break;
    case 3: // ECALL / csrrw
        // inspect funct3 field
        if (funct3 == 0b000) {
            // ECALL
            this->nextPC = mtvec;
            break;
        }
        // csr instruction
        this->nextPC += 4;
        break;
    case 4: // Next instruction
        this->nextPC += 4;
        break;
    }
    // Check for external interrupts or exceptions
    if(interrupt_taken) {
        this->nextPC = mtvec;
    }
}

template <typename T>
T NextPC<T>::getNextPC()
{
    return this->nextPC;
}

template class NextPC<u_int32_t>;
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <typeinfo>
#include <exception>
#include <math.h>
#include "Components.h"

// 64-bit ALU Not Yet Supported
ALU::ALU()
{
    this->size_of_operand = 32;
    this->MSB = 0;
    this->isPositiveA = 0;
    this->isPositiveB = 0;
}

// 32-bit ALU Operations
uint32_t ALU::exec(uint32_t A, uint32_t B, int ALU_op)
{
    switch (ALU_op) {
    case ADD: // add
        // C++ implicitly interprets uint32_t operations as 2s compliment
        // The hardware implementation of 32-bit adder is up to the user
        // Current Implementation based on Ripple or Carry Lookahead
        return A + B;
    case SUB: // sub
        // A - B = A + ~B
        // Invert the input of B and use the 32-bit adder hardware
        return A - B;
    case OR: // or
        // 32-bit OR Gate
        return A | B;
    case AND: // and
        // 32-bit AND Gate
        return A & B;
    case XOR: // xor
        // 32-bit XOR Gate
        return A ^ B;
    case SRL: // srl
        // 32-bit Right Shift Register
        return A >> B;
    case SRA: // sra
        // 32-bit Right Shifter Register with Sign Extension
        this->MSB = -(A >> (this->size_of_operand - 1));
	    return (this->MSB ^ A) >> B ^ this->MSB;
    case SLL: // sll
        // 32-bit Left Shift Register
        return A << B;
    case SLT: // slt
        // In hardware, use MSB output of 32-bit subtractor (which uses 32-bit adder)
        this->MSB = (A - B) >> (this->size_of_operand - 1);
        return this->MSB == 1;
    case SLTU: // sltu
        this->isPositiveA = ((A >> (this->size_of_operand - 1)) & 0x1) == 0;
        this->isPositiveB = ((B >> (this->size_of_operand - 1)) & 0x1) == 0;
        // If a and b are both positive or both negative
        if ((this->isPositiveA && this->isPositiveB) || (!this->isPositiveA && !this->isPositiveB)) {
            return ((A - B) >> (this->size_of_operand - 1)) == 1;
        }
        // [A is negative, B is positive] or [A is positive, B is negative]
        return !this->isPositiveA && this->isPositiveB ? 0 : 1;
    default: // Invalid opcode
        std::cerr << "Invalid ALU Opcode Exception" << std::endl;
        exit(1);
    }
}

class ALU;
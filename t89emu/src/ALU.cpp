#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <exception>
#include <math.h>
#include "Components.h"

// 32-bit ALU Operations
uint32_t ALU::execute(uint32_t A, uint32_t B, int ALU_op) 
{
    uint32_t MSB;
    
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
        MSB = -(A >> 31);
	    return (MSB ^ A) >> B ^ MSB;
    case SLL: // sll
        // 32-bit Left Shift Register
        return A << B;
    case SLT: // slt
        // In hardware, use MSB output of 32-bit subtractor (which uses 32-bit adder)
        MSB = (A - B) >> 31;
        return (MSB == 1);
    case SLTU: // sltu
        return (A < B); // Unsigned integer comparison
    default: // Invalid opcode
        std::cerr << "Invalid ALU Opcode Exception" << std::endl;
        exit(EXIT_FAILURE);
    }
}
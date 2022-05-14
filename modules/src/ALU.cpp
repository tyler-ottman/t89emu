#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <typeinfo>
#include <exception>
#include <math.h>
#include "Components.h"

// C++ implicitly interprets uint32_t operations as 2s compliment
// The hardware implementation of 32-bit adder is up to the user
// Current Implementation based on Ripple or Carry Lookahead
uint32_t ALU::_add()
{
    return this->A + this->B;
}

// A - B = A + ~B
// Invert the input of B and use the 32-bit adder hardware
uint32_t ALU::_sub()
{
    return this->A - this->B;
}

// 32-bit OR Gate
uint32_t ALU::_or()
{
    return this->A | this->B;
}

// 32-bit AND Gate
uint32_t ALU::_and()
{
    return this->A & this->B;
}

// 32-bit XOR Gate
uint32_t ALU::_xor()
{
    return this->A ^ this->B;
}

// 32-bit Right Shift Register
uint32_t ALU::_srl()
{
    uint32_t shamt = this->B % 32;
    return this->A >> shamt;
}

// 32-bit Right Shifter Register with Sign Extension
uint32_t ALU::_sra()
{
    uint32_t alu_out = this->A;
    for (uint32_t i = 0; i < (this->B % (uint32_t)this->size_of_operand); i++) {
		uint32_t MSB = alu_out & 0x80000000;
		alu_out >>= 1;
		if (MSB) // Sign Extension to negative numbers
			alu_out |= 0x80000000;
	}
    return alu_out;
}

// 32-bit Left Shift Register
uint32_t ALU::_sll()
{
    return this->A << this->B;
}

// In hardware, use MSB output of 32-bit subtractor (which uses 32-bit adder)
uint32_t ALU::_slt()
{
    uint32_t alu_out = this->A - this->B;
    uint32_t MSB = alu_out >> (this->size_of_operand - 1);
    return MSB == 1;
}

uint32_t ALU::_sltu()
{
    uint32_t isPositiveA = ((this->A >> (this->size_of_operand - 1)) & 0x1) == 0;
    uint32_t isPositiveB = ((this->B >> (this->size_of_operand - 1)) & 0x1) == 0;
    // If a and b are both positive or both negative
    if ((isPositiveA && isPositiveB) || (!isPositiveA && !isPositiveB)) {
        return ((this->A - this->B) >> (this->size_of_operand - 1)) == 1;
    }
    // [A is negative, B is positive] or [A is positive, B is negative]
    return !isPositiveA && isPositiveB ? 0 : 1;
}

// 64-bit ALU Not Yet Supported
ALU::ALU()
{
    this->size_of_operand = 32;
}

void ALU::printOperands()
{
    std::cout << "A: " << this->A << "\nB: " << this->B << std::endl;
}

/* ALU Can perform 8/16/32/64-bit operations
ALU Operations (ALU_op)
     0: add
     1: sub
     2: or
     3: and
     4: xor
     5: srl
     6: sra
     7: sll
     8: slt
     9: sltu
*/
uint32_t ALU::exec(uint32_t A, uint32_t B, int ALU_op)
{
    // Set operands
    this->A = A;
    this->B = B;

    switch (ALU_op) {
    case (0): // add
        return _add();
    case (1): // sub
        return _sub();
    case (2): // or
        return _or();
    case (3): // and
        return _and();
    case (4): // xor
        return _xor();
    case (5): // srl
        return _srl();
    case (6): // sra
        return _sra();
    case (7): // sll
        return _sll();
    case (8): // slt
        return _slt();
    case (9): // sltu
        return _sltu();
    default: // Invalid opcode
        std::cerr << "Invalid ALU Opcode Exception" << std::endl;
        exit(1);
    }
}

class ALU;
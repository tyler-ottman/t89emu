#include "MemControlUnit.h"

MemControlUnit::MemControlUnit() {

}

MemControlUnit::~MemControlUnit() {
    
}

// # of bytes written to memory
int MemControlUnit::getMemSize(uint32_t funct3) {
    switch(funct3) {
    case 0b000: // LB, SB 
    case 0b100: // LBU
        return BYTE;
    case 0b001: // LH, SH
    case 0b101: // LHU
        return HALFWORD;
    case 0b010: // LW, SW
        return WORD;
    default:
        return 0; // todo: throw exception
    }
}
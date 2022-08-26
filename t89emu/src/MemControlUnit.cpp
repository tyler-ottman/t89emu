#include "Components.h"

// # of bytes written to memory
int MemControlUnit::get_mem_size(uint32_t funct3) {
    switch(funct3) {
        case 0b000: return 1; // LB, SB
        case 0b001: return 2; // LH, SH
        default:    return 4; // LW, LBU, LHU, SW
    }
}
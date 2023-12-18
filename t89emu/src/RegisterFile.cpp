#include <iostream>
#include <stdlib.h>

#include "RegisterFile.h"

/*
x0	        zero
x1	        ra
x2	        sp
x3	        gp
x4	        tp
x5-7        t0-2
x8	        s0
x9	        s1
x10-11	    a0-1
x12-17	    a2-7
x18-27	    s2-11
x28-31	    t3-6
*/
RegisterFile::RegisterFile() {
    registers = new uint32_t[32]();
}

RegisterFile::~RegisterFile() {
    delete registers;
}

uint32_t RegisterFile::read(int reg) {
    return registers[reg];
}

void RegisterFile::write(uint32_t data, int reg) {
    // Writing to register x0 should do nothing
    registers[reg] = (reg != 0) ? data : 0;
}
#include <iostream>

#include "ProgramCounter.h"

ProgramCounter::ProgramCounter() {
    pc = 0;
}

void ProgramCounter::setPc(uint32_t nextPc) {
    pc = nextPc;
}

uint32_t ProgramCounter::getPc() {
    return pc;
}

uint32_t *ProgramCounter::getPcPtr() {
    return &pc;
}
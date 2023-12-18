#include <iostream>

#include "Trap.h"

Trap::Trap() {
    
}

Trap::~Trap() {

}

void Trap::takeTrap(Csr* csr, ProgramCounter* pc, NextPc* nextPc, uint32_t mcause) {
    // Save current PC to machine exception program counter
    csr->mepc = pc->getPc();

    // If ECALL save PC + 4
    if (mcause == ECALL_FROM_M_MODE) {
        csr->mepc += 4;
    }
    
    // Write cause of exception/interrupt to mcause
    csr->mcause = mcause;

    // If MIE was enabled, store in MPIE
    if (csr->getMie()) {
        csr->setMpie();
    }

    // Globally disable interrupts
    csr->resetMie();

    // Save previous privilege level in MPP
    csr->setMpp(MACHINE_MODE);

    csr->resetMie();   // Globally disable interrupts

    // Set PC to jump to proper interrupt/exception handler in vector table
    uint32_t vector_table_offset;

    if (mcause >= 0x80000000) {
        // Interrupt
        vector_table_offset = mcause - 0x80000000;
    } else {
        // Exception
        vector_table_offset = 16 + mcause;
    }

    nextPc->nextPc = csr->mtvec + 4 * vector_table_offset;
    pc->setPc(csr->mtvec + 4 * vector_table_offset);
    // printf("trapping to PC: %08x\n", pc->PC);
}
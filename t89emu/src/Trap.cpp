#include "Trap.h"

Trap::Trap() {
    
}

Trap::~Trap() {

}

void Trap::take_trap(CSR* csr, ProgramCounter* pc, NextPC* nextpc, uint32_t mcause) {
    // Save current PC to machine exception program counter
    csr->mepc = pc->PC;
    
    // If MIE was enabled, store in MPIE
    if (csr->get_mie()) {
        csr->set_mpie();
    }

    // Globally disable interrupts
    csr->reset_mie();

    // Save previous privilege level in MPP
    csr->set_mpp(MACHINE_MODE);

    csr->reset_mie();   // Globally disable interrupts

    // Set PC to jump to proper interrupt/exception handler in vector table
    uint32_t vector_table_offset = (mcause >= 0x80000000) ? (mcause - 0x80000000) : mcause;
    nextpc->nextPC = csr->mtvec + 4 * vector_table_offset;
    pc->PC = csr->mtvec + 4 * vector_table_offset;
}
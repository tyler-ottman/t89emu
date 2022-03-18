#include <iostream>
#include "Components.h"

uint32_t mie = 0;              // Machine interrupt enable bit
uint32_t mepc = 0;             // Machine exception pc
uint32_t mtvec = 0;            // Machine trap vector
uint32_t csr_mcause = 0;           // Machine trap cause
uint32_t machine_mode = 1; // Machine mode bit
uint32_t csr_rd = 0;               // register destination

CSR::CSR(){
    this->mie = 0;
    this->mepc = 0;
    this->mtvec = 0;
    this->csr_mcause = 0;
    this->machine_mode = 1;
    this->csr_rd = 0;
}

// 0x000: mie
// 0x001: mepc
// 0x002: mtvec
// 0x003: mcause
// 0x004: machine
// 0x005: mtime
// 0x006: mtimecmp_low
// 0x007: mtimecmp_high
// intr_taken, csr_addr, csr_we, mcause, pc, wd
void CSR::update_csr(uint32_t intr_taken, uint32_t csr_addr, uint32_t csr_we, uint32_t mcause, uint32_t pc, uint32_t wd)
{
    // Check if updating CSR
    if (csr_we && this->machine_mode)
    {
        switch (csr_addr)
        {
        case 0x000: // mie
            this->mie = wd;
            break;
        case 0x001: // mepc
            this->mepc = wd;
            break;
        case 0x002: // mtvec
            this->mtvec = wd;
            break;
        case 0x003: // mcause
            this->csr_mcause = wd;
            break;
        case 0x004: // machine mode
            this->machine_mode = wd;
            break;
        }
    }

    // Check if interrupt
    if (intr_taken)
    {
        // update mcause, machine mode, mepc
        this->csr_mcause = mcause;
        this->machine_mode = 1;
        this->mepc = pc;
    }
}

uint32_t CSR::get_csr(int addr)
{
    switch (addr)
    {
    case 0x000: // mie
        return this->mie;
    case 0x001: // mepc
        return this->mepc;
    case 0x002: // mtvec
        return this->mtvec;
    case 0x003: // mcause
        return this->csr_mcause;
    case 0x004: // machine mode
        return this->machine_mode;
    }
    return 0;
}

class CSR;
#include <iostream>
#include "Components.h"

CSR::CSR(){
    // Supported ISA
    // I extension
    misa = (1 << 8);

    // Fields not implemented
    mvendorid = 0;
    marchid = 0;
    mhartid = 0;
    mimpid = 0;

    // Global Interrupts enabled by default
    mstatus = (1 << 3);

    mtvec = 0; // Loaded by software
    mie = 0; // Enable all interrupts in software
    mip = 0;
    mcause = 0;
    mepc = 0;
    mscratch = 0;
    mtval = 0;



    mcycles = 0;
}

uint32_t CSR::read_csr(uint32_t address) {
    switch(address) {
        case MSTATUS:   return mstatus;
        case MISA:      return misa;
        case MTVEC:     return mtvec;
        case MIE:       return mie;
        case MIP:       return mip;
        case MCAUSE:    return mcause;
        case MEPC:      return mepc;
        case MSCRATCH:  return mscratch;
        case MTVAL:     return mtval;
        default:
            std::cerr << "Unknown CSR" << std::endl;
            exit(EXIT_FAILURE);
    }
}

void CSR::write_csr(uint32_t address, uint32_t data) {
    switch(address) {
        case MSTATUS:   mcause = data; break;
        case MISA:      misa = data; break;
        case MTVEC:     mtvec  = data; break;
        case MIE:       mie = data; break;
        case MIP:       mip = data; break;
        case MCAUSE:    mcause = data; break;
        case MEPC:      mepc = data; break;
        case MSCRATCH:  mscratch = data; break;
        case MTVAL:     mtval = data; break;
        default:
            std::cerr << "Unknown CSR" << std::endl;
            exit(EXIT_FAILURE);
    }
}

class CSR;
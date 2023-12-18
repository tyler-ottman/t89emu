#include <iostream>
#include "Csr.h"

Csr::Csr(){
    // Supported ISA
    // I extension
    misa = 0;

    // Fields not implemented
    mvendorid = 0;
    marchid = 0;
    mhartid = 0;
    mimpid = 0;

    // Enable by software
    mstatus = 0;

    // Loaded by software
    mtvec = 0;
    
    // Enable all interrupts in software
    mie = 0;
    mip = 0;

    mcause = 0;
    mepc = 0;
    mscratch = 0;

    // Possibly no implementation
    mtval = 0;
}

uint32_t Csr::readCSR(uint32_t address) {
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

void Csr::writeCSR(uint32_t address, uint32_t data) {
    switch(address) {
        case MSTATUS:   mstatus = data; break;
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
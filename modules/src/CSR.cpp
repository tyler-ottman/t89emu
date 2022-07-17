#include <iostream>
#include "Components.h"

CSR::CSR(){
    // Supported ISA
    // U extension, S extension, N extension, I extension
    misa = (1 << 20) | (1 << 18) | (1 << 13) | (1 << 8);
    mstatus = 0;
    mtvec = 0;
    mie = 0;
    mip = 0;
    mcause = 0;
    mepc = 0;
    mscratch = 0;
    mtval = 0;
    mcycles = 0;
}

uint32_t CSR::read_csr(uint32_t address) {
    switch(address) {
        case MSTATUS:   return this->mstatus;
        case MISA:      return misa;
        case MTVEC:     return this->mtvec;
        case MIE:       return this->mie;
        case MIP:       return this->mip;
        case MCAUSE:    return this->mcause;
        case MEPC:      return this->mepc;
        case MSCRATCH:  return this->mscratch;
        case MTVAL:     return this->mtval;
        default:
            std::cerr << "Unknown CSR" << std::endl;
            exit(EXIT_FAILURE);
    }
}

void CSR::write_csr(uint32_t address, uint32_t data) {
    switch(address) {
        case MSTATUS:   this->mcause = data; break;
        case MISA:      misa = data; break;
        case MTVEC:     this->mtvec  = data; break;
        case MIE:       this->mie = data; break;
        case MIP:       this->mip = data; break;
        case MCAUSE:    this->mcause = data; break;
        case MEPC:      this->mepc = data; break;
        case MSCRATCH:  this->mscratch = data; break;
        case MTVAL:     this->mtval = data; break;
        default:
            std::cerr << "Unknown CSR" << std::endl;
            exit(EXIT_FAILURE);
    }
}

class CSR;
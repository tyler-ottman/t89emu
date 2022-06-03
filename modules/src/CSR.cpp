#include <iostream>
#include "Components.h"

CSR::CSR(){
    this->mstatus = 0;
    this->mtvec = 0; // To be decided
    this->mie = 0;
    this->mip = 0;
    this->mcause = 0;
    this->mepc = 0;
    this->mscratch = 0;
    this->mtval = 0;
    this->mcycles = 0;
}

uint32_t CSR::read_csr(uint32_t address) {
    switch(address) {
        case MSTATUS:   return this->mstatus;
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
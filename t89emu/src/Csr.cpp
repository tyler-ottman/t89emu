#include <iostream>
#include "Csr.h"

Csr::Csr() {
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

Csr::~Csr() {

}

uint32_t Csr::readCsr(uint32_t address) {
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

void Csr::writeCsr(uint32_t address, uint32_t data) {
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

void Csr::setMie() {
    mstatus |= (1 << MSTATUS_MIE_MASK);
}

void Csr::setMpie() {
    mstatus |= (1 << MSTATUS_MPIE_MASK);
}

void Csr::setMpp(int mask) {
    mstatus |= ((mask & 0b11) << MSTATUS_MPP_MASK);
}

void Csr::setMeip() {
    mip |= (1 << MIP_MEIP_MASK);
}

void Csr::setMtip() {
    mip |= (1 << MIP_MTIP_MASK);
}

void Csr::setMsip() {
    mip |= (1 << MIP_MSIP_MASK);
}

void Csr::resetMie() {
    mstatus &= ~(1 << MSTATUS_MIE_MASK);
}

void Csr::resetMpie() {
    mstatus &= ~(1 << MSTATUS_MPIE_MASK);
}

void Csr::resetMpp() {
    mstatus &= ~(0b11 << MSTATUS_MPP_MASK);
}

void Csr::resetMeip() {
    mip &= ~(1 << MIP_MEIP_MASK);
}

void Csr::resetMtip() {
    mip &= ~(1 << MIP_MTIP_MASK);
}

void Csr::resetMsip() {
    mip &= ~(1 << MIP_MSIP_MASK);
}

uint32_t Csr::getMie() {
    return ((mstatus >> MSTATUS_MIE_MASK) & 0b1);
}

uint32_t Csr::getMpie() {
    return ((mstatus >> MSTATUS_MPIE_MASK) & 0b1);
}

uint32_t Csr::getMpp() {
    return ((mstatus >> MSTATUS_MPP_MASK) & 0b11);
}

uint32_t Csr::getMeie() {
    return ((mie >> MIE_MEIE_MASK) &0b1);
}

uint32_t Csr::getMtie() {
    return ((mie >> MIE_MTIE_MASK) &0b1);
}

uint32_t Csr::getMsie() {
    return ((mie >> MIE_MSIE_MASK) &0b1);
}

uint32_t Csr::getMeip() {
    return ((mip >> MIP_MEIP_MASK) &0b1);
}

uint32_t Csr::getMtip() {
    return ((mip >> MIP_MTIP_MASK) &0b1);
}

uint32_t Csr::getMsip() {
    return ((mip >> MIP_MSIP_MASK) &0b1);
}

void Csr::setMepc(uint32_t mepc) {
    this->mepc = mepc;
}

void Csr::setMcause(uint32_t mcause) {
    this->mcause = mcause;
}

uint32_t Csr::getMepc() {
    return mepc;
}

uint32_t Csr::getMtvec() {
    return mtvec;
}
#include <iostream>
#include "Csr.h"

Csr::Csr() {
    // Machine Instruction Set Architecture (I)
    registers[CSR_MISA] = 0;

    // Machine Vendor ID (no impl)
    registers[CSR_MVENDORID] = 0;

    // Machine Architecture ID (no impl)
    registers[CSR_MARCHID] = 0;

    // Machine Implementation ID (no impl)
    registers[CSR_MIMPID] = 0;

    // Machine Hart ID
    registers[CSR_MHARTID] = 0;

    // Machine Status
    registers[CSR_MSTATUS] = 0;

    // Machine Trap-Vector Base-Address
    registers[CSR_MTVEC] = 0;

    // Machine Interrupt Enable
    registers[CSR_MIE] = 0;

    // Machine Interrupt Pending
    registers[CSR_MIP] = 0;
    
    // Machine Cause
    registers[CSR_MCAUSE] = 0;

    // Machine Exception Program Counter
    registers[CSR_MEPC] = 0;

    // Machine Scratch
    registers[CSR_MSCRATCH] = 0;

    // Machine Trap Value
    registers[CSR_MTVAL] = 0;
}

Csr::~Csr() {

}

uint32_t Csr::readCsr(uint32_t address) {
    return registers[address];
}

void Csr::writeCsr(uint32_t address, uint32_t data) {
    registers[address] = data;
}

void Csr::setMie() {
    registers[CSR_MSTATUS] |= (1 << MSTATUS_MIE_MASK);
}

void Csr::setMpie() {
    registers[CSR_MSTATUS] |= (1 << MSTATUS_MPIE_MASK);
}

void Csr::setMpp(int mask) {
    registers[CSR_MSTATUS] |= ((mask & 0b11) << MSTATUS_MPP_MASK);
}

void Csr::setMeip() {
    registers[CSR_MIP] |= (1 << MIP_MEIP_MASK);
}

void Csr::setMtip() {
    registers[CSR_MIP] |= (1 << MIP_MTIP_MASK);
}

void Csr::setMsip() {
    registers[CSR_MIP] |= (1 << MIP_MSIP_MASK);
}

void Csr::resetMie() {
    registers[CSR_MSTATUS] &= ~(1 << MSTATUS_MIE_MASK);
}

void Csr::resetMpie() {
    registers[CSR_MSTATUS] &= ~(1 << MSTATUS_MPIE_MASK);
}

void Csr::resetMpp() {
    registers[CSR_MSTATUS] &= ~(0b11 << MSTATUS_MPP_MASK);
}

void Csr::resetMeip() {
    registers[CSR_MIP] &= ~(1 << MIP_MEIP_MASK);
}

void Csr::resetMtip() {
    registers[CSR_MIP] &= ~(1 << MIP_MTIP_MASK);
}

void Csr::resetMsip() {
    registers[CSR_MIP] &= ~(1 << MIP_MSIP_MASK);
}

uint32_t Csr::getMie() {
    return ((registers[CSR_MSTATUS] >> MSTATUS_MIE_MASK) & 0b1);
}

uint32_t Csr::getMpie() {
    return ((registers[CSR_MSTATUS] >> MSTATUS_MPIE_MASK) & 0b1);
}

uint32_t Csr::getMpp() {
    return ((registers[CSR_MSTATUS] >> MSTATUS_MPP_MASK) & 0b11);
}

uint32_t Csr::getMeie() {
    return ((registers[CSR_MIE] >> MIE_MEIE_MASK) &0b1);
}

uint32_t Csr::getMtie() {
    return ((registers[CSR_MIE] >> MIE_MTIE_MASK) &0b1);
}

uint32_t Csr::getMsie() {
    return ((registers[CSR_MIE] >> MIE_MSIE_MASK) &0b1);
}

uint32_t Csr::getMeip() {
    return ((registers[CSR_MIP] >> MIP_MEIP_MASK) &0b1);
}

uint32_t Csr::getMtip() {
    return ((registers[CSR_MIP] >> MIP_MTIP_MASK) &0b1);
}

uint32_t Csr::getMsip() {
    return ((registers[CSR_MIP] >> MIP_MSIP_MASK) &0b1);
}

void Csr::setMepc(uint32_t mepc) {
    registers[CSR_MEPC] = mepc;
}

void Csr::setMcause(uint32_t mcause) {
    registers[CSR_MCAUSE] = mcause;
}

uint32_t Csr::getMepc() {
    return registers[CSR_MEPC];
}

uint32_t Csr::getMtvec() {
    return registers[CSR_MTVEC];
}

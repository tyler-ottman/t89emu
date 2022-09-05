#include <iostream>
#include "ClintMemoryDevice.h"

ClintMemoryDevice::ClintMemoryDevice(uint32_t base, uint32_t size) {
    baseAddress = base;
    deviceSize = size;
    interrupt_type = 0;

    mem = new uint8_t[deviceSize]();
}

uint32_t ClintMemoryDevice::read(uint32_t addr, uint32_t size, uint32_t* read_value) {
    // Check memory access is aligned
    if (!check_alignment(addr, size)) {
        return LOAD_ADDRESS_MISALIGNED;
    }

    // Invalid memory size access handled by memory control unit
    switch (size) {
    case BYTE: *read_value = *((uint8_t*)get_address(addr)); break;
    case HALFWORD: *read_value = *((uint16_t*)get_address(addr)); break;
    case WORD: *read_value = *((uint32_t*)get_address(addr)); break;
    }

    return STATUS_OK;
}

uint32_t ClintMemoryDevice::write(uint32_t addr, uint32_t value, uint32_t size) {
    // Check memory access is aligned
    if (!check_alignment(addr, size)) {
        return STORE_ADDRESS_MISALIGNED;
    }
    
    switch (size) {
    case BYTE: *((uint8_t*)get_address(addr)) = value; break;
    case HALFWORD: *((uint16_t*)get_address(addr)) = value; break;
    case WORD : *((uint32_t*)get_address(addr)) = value; break;
    }
    
    return STATUS_OK;
}

void ClintMemoryDevice::next_cycle(CSR* csr) {
    // Increment 64-bit mcycle
    uint64_t* mcycle = (uint64_t*)&mem[MCYCLE_OFFSET];
    (*mcycle)++;

    // Check for Timer interrupts
    // When mcycle >= mtimecmp, set pending timer interrupt
    uint64_t* mtimecmp = (uint64_t*)&mem[MTIMECMP_OFFSET];
    if (*mcycle >= *mtimecmp) csr->set_mtip();
    else csr->reset_mtip();

    // Check for software interrupts, if MSIP is 1
    // write MSIP to field designed in MIP register
    uint32_t* msip = (uint32_t*)&mem[MSIP_OFFSET];
    if (*msip == 1) csr->set_msip();
}

bool ClintMemoryDevice::check_interrupts(CSR* csr) {
    // Verify Global Interrupts are enabled via MIA bit
    // Later: Implement interrupt checks in S-mode
    if (!csr->get_mie()) {
        return false;
    }

    // Check for software interrupts
    uint32_t* msip = (uint32_t*)&mem[MSIP_OFFSET];
    if ((*msip == 1) && csr->get_msie()) {
        interrupt_type = MACHINE_SOFTWARE_INTERRUPT;
        return true;
    }

    // Check for timer interrupts
    if (csr->get_mtip() && csr->get_mtie()) {
        interrupt_type = MACHINE_TIMER_INTERRUPT;
        return true;
    }

    return false;
}
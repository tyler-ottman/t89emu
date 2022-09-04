#include <iostream>
#include "ClintMemoryDevice.h"

ClintMemoryDevice::ClintMemoryDevice(uint32_t base, uint32_t size) {
    baseAddress = base;
    deviceSize = size;
    interrupt_type = 0;

    mem = new uint8_t[deviceSize]();
}

uint32_t ClintMemoryDevice::read(uint32_t addr, uint32_t size) {
    switch (size) {
    case BYTE: return *((uint8_t*)get_address(addr));
    case HALFWORD: return *((uint16_t*)get_address(addr));
    case WORD: return *((uint32_t*)get_address(addr));
    default:
        // Insert excepteion for trap handler later
        return 0;
        break;
    }
}

void ClintMemoryDevice::write(uint32_t addr, uint32_t value, uint32_t size) {
    switch (size) {
    case BYTE: *((uint8_t*)get_address(addr)) = value; break;
    case HALFWORD: *((uint16_t*)get_address(addr)) = value; break;
    case WORD : *((uint32_t*)get_address(addr)) = value; break;
    default:
        // Insert exception for trap handler later
        break;
    }
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
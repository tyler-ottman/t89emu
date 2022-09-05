#include "ROMMemoryDevice.h"
#include <iostream>

ROMMemoryDevice::ROMMemoryDevice(uint32_t base, uint32_t size) {
    baseAddress = base;
    deviceSize = size;

    mem = new uint8_t[deviceSize]();
}

uint32_t ROMMemoryDevice::read(uint32_t addr, uint32_t size, uint32_t* read_value) {
    // Check memory access is aligned
    if (!check_alignment(addr, size)) {
        return LOAD_ADDRESS_MISALIGNED;
    }

    switch (size) {
    case BYTE: *read_value = *((uint8_t*)get_address(addr));
    case HALFWORD: *read_value = *((uint16_t*)get_address(addr));
    case WORD: *read_value = *((uint32_t*)get_address(addr));
    }
    
    return STATUS_OK;
}

uint32_t ROMMemoryDevice::write(uint32_t addr, uint32_t value, uint32_t size) {
    // Cannot write to read only memory, throw exception
    return ILLEGAL_INSTRUCTION;
}
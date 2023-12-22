#include "RomMemoryDevice.h"

#include <iostream>

RomMemoryDevice::RomMemoryDevice(uint32_t base, uint32_t size)
    : MemoryDevice::MemoryDevice(base, size) {
    
}

uint32_t RomMemoryDevice::read(uint32_t addr, uint32_t size,
                               uint32_t *readValue) {
    // Check memory access is aligned
    if (!checkAlignment(addr, size)) {
        return LOAD_ADDRESS_MISALIGNED;
    }

    switch (size) {
    case BYTE: *readValue = *((uint8_t *)getAddress(addr));
    case HALFWORD: *readValue = *((uint16_t *)getAddress(addr));
    case WORD: *readValue = *((uint32_t *)getAddress(addr));
    }
    
    return STATUS_OK;
}

uint32_t RomMemoryDevice::write(uint32_t addr, uint32_t writeValue,
                                uint32_t size) {
    // Cannot write to read only memory, throw exception
    return ILLEGAL_INSTRUCTION;
}
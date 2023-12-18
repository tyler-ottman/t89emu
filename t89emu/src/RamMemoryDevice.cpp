#include "RamMemoryDevice.h"

#include <iostream>

RamMemoryDevice::RamMemoryDevice(uint32_t base, uint32_t size) {
    baseAddress = base;
    deviceSize = size;

    mem = new uint8_t[deviceSize]();
}

uint32_t RamMemoryDevice::read(uint32_t addr, uint32_t size, uint32_t* readValue) {
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

uint32_t RamMemoryDevice::write(uint32_t addr, uint32_t writeValue, uint32_t size) {
    // Check memory access is aligned
    if (!checkAlignment(addr, size)) {
        return STORE_ADDRESS_MISALIGNED;
    }

    switch (size) {
    case BYTE: *((uint8_t *)getAddress(addr)) = writeValue; break;
    case HALFWORD: *((uint16_t *)getAddress(addr)) = writeValue; break;
    case WORD : *((uint32_t *)getAddress(addr)) = writeValue; break;
    }
    
    return STATUS_OK;
}
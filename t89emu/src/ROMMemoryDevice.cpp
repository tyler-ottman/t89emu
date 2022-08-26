#include "Components.h"
#include <iostream>

ROMMemoryDevice::ROMMemoryDevice(uint32_t base, uint32_t size) {
    baseAddress = base;
    deviceSize = size;

    mem = new uint8_t[deviceSize]();
}

uint32_t ROMMemoryDevice::read(uint32_t addr, uint32_t size) {
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

void ROMMemoryDevice::write(uint32_t addr, uint32_t value, uint32_t size) {
    // Cannot write to read only memory, throw exception
}
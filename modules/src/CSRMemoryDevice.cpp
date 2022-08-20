#include "Components.h"

CSRMemoryDevice::CSRMemoryDevice(uint32_t base, uint32_t size) {
    baseAddress = base;
    deviceSize = size;

    mem = new uint8_t[deviceSize]();
}

uint32_t CSRMemoryDevice::read(uint32_t addr, uint32_t size) {
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

void CSRMemoryDevice::write(uint32_t addr, uint32_t value, uint32_t size) {
    switch (size) {
    case BYTE: *((uint8_t*)get_address(addr)) = value; break;
    case HALFWORD: *((uint16_t*)get_address(addr)) = value; break;
    case WORD : *((uint32_t*)get_address(addr)) = value; break;
    default:
        // Insert exception for trap handler later
        break;
    }
}
#include "Components.h"

MemoryDevice::MemoryDevice(uint32_t base, uint32_t size) {
    baseAddress = base;
    deviceSize = size;

    mem = new uint8_t(deviceSize);
}

MemoryDevice::~MemoryDevice() {
    delete mem;
}

uint8_t* MemoryDevice::get_address(uint32_t addr) {
    uint32_t addr_offset = addr - baseAddress;
    return (uint8_t*)(mem + addr_offset);
}

uint32_t MemoryDevice::read(uint32_t addr, uint32_t size) {
    switch (size) {
    case BYTE: return *((uint8_t*)get_address(addr));
    case HALFWORD: return *((uint16_t*)get_address(addr));
    case WORD: return *((uint32_t*)get_address(addr));
    default:
        // Insert excepteion for trap handler later
        break;
    }
}

void MemoryDevice::write(uint32_t addr, uint32_t value, uint32_t size) {
    switch (size) {
    case BYTE: *((uint8_t*)get_address(addr)) = value; break;
    case HALFWORD: *((uint16_t*)get_address(addr)) = value; break;
    case WORD : *((uint32_t*)get_address(addr)) = value; break;
    default:
        // Insert exception for trap handler later
        break;
    }
}
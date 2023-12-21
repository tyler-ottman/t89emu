#include "MemoryDevice.h"

MemoryDevice::MemoryDevice() {

}

MemoryDevice::~MemoryDevice() {
    delete mem;
}

bool MemoryDevice::checkAlignment(uint32_t addr, uint32_t size) {
    switch (size) {
    case WORD: return (addr % WORD == 0); // Align on 4 bytes
    case HALFWORD: return (addr % HALFWORD == 0); // Align on 2 bytes
    case BYTE: return true; // Byte accesses always aligned
    }
    
    return true; // Should never reach hear
}

uint8_t *MemoryDevice::getAddress(uint32_t addr) {
    uint32_t addr_offset = addr - baseAddress;
    return (uint8_t *)(mem + addr_offset);
}

uint32_t MemoryDevice::getDeviceSize() {
    return deviceSize;
}

uint8_t *MemoryDevice::getBuffer() {
    return mem;
}
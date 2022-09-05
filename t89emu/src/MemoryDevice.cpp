#include "MemoryDevice.h"

MemoryDevice::MemoryDevice() {

}

MemoryDevice::~MemoryDevice() {
    delete mem;
}

uint8_t* MemoryDevice::get_address(uint32_t addr) {
    uint32_t addr_offset = addr - baseAddress;
    return (uint8_t*)(mem + addr_offset);
}

bool MemoryDevice::check_alignment(uint32_t addr, uint32_t size) {
    switch (size) {
    case WORD: return (addr % WORD == 0); // Align on 4 bytes
    case HALFWORD: return (addr % HALFWORD == 0); // Align on 2 bytes
    case BYTE: return true; // Byte accesses always aligned
    }
    
    return true; // Should never reach hear
}
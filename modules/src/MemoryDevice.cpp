#include "Components.h"

// MemoryDevice::MemoryDevice(uint32_t base, uint32_t size) {
//     baseAddress = base;
//     deviceSize = size;

//     mem = new uint8_t(deviceSize);
// }
#include <iostream>
MemoryDevice::MemoryDevice() {

}

MemoryDevice::~MemoryDevice() {
    delete mem;
}

uint8_t* MemoryDevice::get_address(uint32_t addr) {
    uint32_t addr_offset = addr - baseAddress;
    return (uint8_t*)(mem + addr_offset);
}
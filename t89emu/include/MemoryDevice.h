#include <stdint.h>
#include "Architecture.h"

#ifndef MEMORYDEVICE_H
#define MEMORYDEVICE_H

class MemoryDevice {
public:
    //MemoryDevice(uint32_t, uint32_t);
    MemoryDevice();
    virtual ~MemoryDevice();
    virtual uint8_t* get_address(uint32_t);
    virtual uint32_t read(uint32_t, uint32_t) = 0;
    virtual void write(uint32_t, uint32_t, uint32_t) = 0;
    uint32_t baseAddress;
    uint32_t deviceSize;
    uint8_t* mem;
};

#endif
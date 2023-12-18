#include <stdint.h>

#include "MemoryDevice.h"

#ifndef ROM_MEMORYDEVICE_H
#define ROM_MEMORYDEVICE_H

// ROM base address defined by linker, maximum possible size is fixed
#define ROM_SIZE 2097152 // 2 MB

class RomMemoryDevice : public MemoryDevice {
public:
    RomMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t addr, uint32_t size, uint32_t *readValue);
    uint32_t write(uint32_t addr, uint32_t writeValue, uint32_t size);
};

#endif // ROM_MEMORYDEVICE_H
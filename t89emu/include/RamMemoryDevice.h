#include <stdint.h>

#include "MemoryDevice.h"

#ifndef RAM_MEMORYDEVICE_H
#define RAM_MEMORYDEVICE_H

// RAM base address defined by linker, maximum possible size is fixed
#define RAM_SIZE 1048576 // 1 MB

class RamMemoryDevice : public MemoryDevice {
public:
    RamMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t addr, uint32_t size, uint32_t *readValue);
    uint32_t write(uint32_t addr, uint32_t writeValue, uint32_t size);
};

#endif // RAM_MEMORYDEVICE_H
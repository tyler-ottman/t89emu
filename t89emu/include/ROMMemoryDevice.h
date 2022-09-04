#include <stdint.h>
#include "MemoryDevice.h"

#ifndef ROM_MEMORYDEVICE_H
#define ROM_MEMORYDEVICE_H

// ROM base address defined by linker, maximum possible size is fixed
#define ROM_SIZE 2097152 // 2 MB

class ROMMemoryDevice : public MemoryDevice {
public:
    ROMMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // ROM_MEMORYDEVICE_H
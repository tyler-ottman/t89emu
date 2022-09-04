#include <stdint.h>
#include "MemoryDevice.h"

#ifndef RAM_MEMORYDEVICE_H
#define RAM_MEMORYDEVICE_H

// RAM base address defined by linker, maximum possible size is fixed
#define RAM_SIZE 1048576 // 1 MB

class RAMMemoryDevice : public MemoryDevice {
public:
    RAMMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // RAM_MEMORYDEVICE_H
#include <stdint.h>
#include "MemoryDevice.h"

#ifndef ROM_MEMORYDEVICE_H
#define ROM_MEMORYDEVICE_H

class ROMMemoryDevice : public MemoryDevice {
public:
    ROMMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // ROM_MEMORYDEVICE_H
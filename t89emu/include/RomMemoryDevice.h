#include <stdint.h>

#include "MemoryDevice.h"

#ifndef ROM_MEMORYDEVICE_H
#define ROM_MEMORYDEVICE_H

class RomMemoryDevice : public MemoryDevice {
public:
    RomMemoryDevice(uint32_t base, uint32_t size);

    uint32_t read(uint32_t addr, uint32_t size, uint32_t *readValue);
    uint32_t write(uint32_t addr, uint32_t writeValue, uint32_t size);
};

#endif // ROM_MEMORYDEVICE_H
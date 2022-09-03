#include <stdint.h>
#include "MemoryDevice.h"

#ifndef CSR_MEMORYDEVICE_H
#define CSR_MEMORYDEVICE_H


class CSRMemoryDevice : public MemoryDevice {
public:
    CSRMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // CSR_MEMORYDEVICE_H
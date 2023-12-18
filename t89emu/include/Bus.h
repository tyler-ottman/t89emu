#include <stdint.h>

#include "Architecture.h"
#include "ClintMemoryDevice.h"
#include "RamMemoryDevice.h"
#include "RomMemoryDevice.h"
#include "VideoMemoryDevice.h"

#ifndef BUS_H
#define BUS_H

// Future: UART
// Future: PLIC
class Bus {
public:
    Bus(uint32_t, uint32_t, uint32_t, uint32_t);
    ~Bus();

    // For read/write, bus and memory devices return exception
    // status codes if something goes wrong
    uint32_t write(uint32_t addr, uint32_t writeValue, uint32_t accessSize);
    uint32_t read(uint32_t addr, uint32_t accessSize, uint32_t *readValue);

    RomMemoryDevice *romDevice;
    RamMemoryDevice *ramDevice;
    VideoMemoryDevice *videoDevice;
    ClintMemoryDevice *clintDevice;
    uint32_t romBase;
    uint32_t romEnd;
    uint32_t ramBase;
    uint32_t ramEnd;
};

#endif // BUS_H
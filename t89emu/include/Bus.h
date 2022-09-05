#include <stdint.h>
#include "Architecture.h"
#include "ROMMemoryDevice.h"
#include "RAMMemoryDevice.h"
#include "VideoMemoryDevice.h"
#include "ClintMemoryDevice.h"

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
    uint32_t write(uint32_t addr, uint32_t write_value, uint32_t access_size);
    uint32_t read(uint32_t addr, uint32_t access_size, uint32_t* read_value);

    ROMMemoryDevice* rom_device;
    RAMMemoryDevice* ram_device;
    VideoMemoryDevice* video_device;
    ClintMemoryDevice* clint_device;
    uint32_t rom_base;
    uint32_t rom_end;
    uint32_t ram_base;
    uint32_t ram_end;
};

#endif // BUS_H
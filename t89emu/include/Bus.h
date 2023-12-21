#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include <vector>

#include "Architecture.h"
#include "ClintMemoryDevice.h"
#include "RamMemoryDevice.h"
#include "RomMemoryDevice.h"
#include "VideoMemoryDevice.h"

class Bus {
public:
    Bus(uint32_t romStart, uint32_t romSize, uint32_t ramStart,
        uint32_t ramSize);
    // Bus(void);
    ~Bus();

    // For read/write, bus and memory devices return exception
    // status codes if something goes wrong
    uint32_t read(uint32_t addr, uint32_t accessSize, uint32_t *readValue);
    uint32_t write(uint32_t addr, uint32_t writeValue, uint32_t accessSize);

    uint32_t getRomBase(void);
    uint32_t getRomEnd(void);
    uint32_t getRamBase(void);
    uint32_t getRamEnd(void);
    
    ClintMemoryDevice *getClintDevice(void);
    RamMemoryDevice *getRamMemoryDevice(void);
    RomMemoryDevice *getRomMemoryDevice(void);
    VideoMemoryDevice *getVideoDevice(void);
    // int addDevice(MemoryDevice *device);

    // MemoryDevice *getDevice(int deviceId);

private:
    uint32_t romBase;
    uint32_t romEnd;
    uint32_t ramBase;
    uint32_t ramEnd;
    
    // TODO: UART, PLIC
    ClintMemoryDevice *clintDevice;
    RamMemoryDevice *ramDevice;
    RomMemoryDevice *romDevice;
    VideoMemoryDevice *videoDevice;

    // std::vector<MemoryDevice *> devices;
};

#endif // BUS_H
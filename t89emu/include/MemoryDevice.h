#include <stdint.h>

#include "Architecture.h"

#ifndef MEMORYDEVICE_H
#define MEMORYDEVICE_H

class MemoryDevice {
public:
    MemoryDevice(void);
    virtual ~MemoryDevice();

    virtual bool checkAlignment(uint32_t addr, uint32_t size);
    virtual uint8_t *getAddress(uint32_t addr);

    // If read was successful, return with STATUS_OK
    // Otherwise read returns with exception code
    virtual uint32_t read(uint32_t addr, uint32_t size,
                          uint32_t *read_value) = 0;

    // If write was successful, return with STATUS_OK
    // Otherwise write returns with exception code
    virtual uint32_t write(uint32_t addr, uint32_t write_value,
                           uint32_t size) = 0;

    uint32_t getBaseAddress(void);
    uint32_t getDeviceSize(void);
    uint8_t *getBuffer(void);

protected:
    uint32_t baseAddress;
    uint32_t deviceSize;
    uint8_t *mem;
};

#endif
#include <stdint.h>

#include "Architecture.h"
#include "Csr.h"
#include "MemoryDevice.h"

#ifndef CLINTMEMORYDEVICE_H
#define CLINTMEMORYDEVICE_H

// Size and offsets do not change across Clint devices
#define CLINT_SIZE              0x18

#define MCYCLE_OFFSET           0x0
#define MTIMECMP_OFFSET         0x8

// Machine software interrupt pending bit must be
// in memory for harts to generate software interrupts
#define MSIP_OFFSET             0x14

class ClintMemoryDevice : public MemoryDevice {
public:
    ClintMemoryDevice(uint32_t base, uint32_t size);
    uint32_t read(uint32_t addr, uint32_t size, uint32_t *read_value);
    uint32_t write(uint32_t addr, uint32_t write_value, uint32_t size);

    void nextCycle(Csr *csr);
    bool checkInterrupts(Csr *csr);

    uint32_t getInterruptType(void);

private:
    uint32_t interruptType; // Used when taking trap
};

#endif // CLINTMEMORYDEVICE_H
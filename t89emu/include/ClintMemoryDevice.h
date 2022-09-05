#include <stdint.h>
#include "Architecture.h"
#include "MemoryDevice.h"
#include "CSR.h"

#ifndef CLINTMEMORYDEVICE_H
#define CLINTMEMORYDEVICE_H

// Clint Device (Memory-mapped CSRs)
#define CLINT_BASE 0x30000000
#define CLINT_SIZE 0x18
#define CLINT_END (CLINT_BASE + CLINT_SIZE)

#define MCYCLE_OFFSET       0
#define MTIMECMP_OFFSET     8

// Machine superviro interrupt pending bit must be 
// in memory for harts to generate software interrupts
#define MSIP_OFFSET         20 // Todo: move keyboard register to plic              

class ClintMemoryDevice : public MemoryDevice {
public:
    ClintMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t addr, uint32_t size, uint32_t* read_value);
    uint32_t write(uint32_t addr, uint32_t write_value, uint32_t size);

    void next_cycle(CSR*);
    bool check_interrupts(CSR*);

    uint32_t interrupt_type; // Used when taking trap
};

#endif // CLINTMEMORYDEVICE_H
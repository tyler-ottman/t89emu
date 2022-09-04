#include <stdint.h>
#include "MemoryDevice.h"
#include "CSR.h"

#ifndef CLINTMEMORYDEVICE_H
#define CLINTMEMORYDEVICE_H

// Interrupt Codes
#define SUPERVISOR_SOFTWARE_INTERRUPT   0X80000001
#define MACHINE_SOFTWARE_INTERRUPT      0x80000003
#define SUPERVISOR_TIMER_INTERRUPT      0x80000005
#define MACHINE_TIMER_INTERRUPT         0x80000007
#define SUPERVISOR_EXTERNAL_INTERRUPT   0x80000009
#define MACHINE_EXTERNAL_INTERRUPT      0x8000000b

// Clint Device (Memory-mapped CSRs)
#define CLINT_BASE 0x30000000
#define CLINT_SIZE 0x14
#define CLINT_END (CLINT_BASE + CLINT_SIZE)

#define MCYCLE_OFFSET       0
#define MTIMECMP_OFFSET     8

// Machine superviro interrupt pending bit must be 
// in memory for harts to generate software interrupts
#define MSIP_OFFSET         20 // Todo: move keyboard register to plic              

class ClintMemoryDevice : public MemoryDevice {
public:
    ClintMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);

    void next_cycle(CSR*);
    bool check_interrupts(CSR*);

    uint32_t interrupt_type; // Used when taking trap
};

#endif // CLINTMEMORYDEVICE_H
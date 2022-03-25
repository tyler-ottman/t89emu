#include <iostream>
#include "Components.h"

#define CSR_REG_SIZE 9
#define MIE 0
#define MPI 1
#define MEPC 2
#define MTVEC 3
#define MCAUSE 4
#define MODE 5
#define MODEP 6

// 0x000: mie               - Machine Interrupt Enable
// 0x001: mpi               - Machine Pending Interrupt Enable
// 0x002: mepc              - Machine Exception Program Counter
// 0x003: mtvec             - Machine Trap Vector
// 0x004: mcause            - Machine Cause (of trap)
// 0x005: mode              - Mode of CPU (user/supervisor/machine)
// 0x006: modep             - Previous Mode of CPU before trap
// 0x007: mtimecmp_low      - Machine Time Compare (lower 32 bits)
// 0x008: mtimecmp_high     - Machine Time Compare (upper 32 bits)
uint32_t* csr_register;

uint32_t csr_we;
uint32_t trap_taken;

CSR::CSR(){
    csr_register = (uint32_t*) calloc(CSR_REG_SIZE, sizeof(uint32_t));
    csr_register[MTVEC] = 0xf0000000;
    csr_register[MODE] = 2; // Machine Initially in machine mode (load OS code)
    this->csr_we = 0;
    this->trap_taken = 0;
}

// Decide when to write to CSR, what happens on interrupt
void CSR::set_control_lines(uint32_t csr_we, uint32_t trap_taken, uint32_t mcause, uint32_t pc) {
    // CSR write enable and trap taken are mutually exclusive
    this->csr_we = csr_we;
    this->trap_taken = trap_taken;
    if(trap_taken) {
        // Trap taken
        csr_register[MCAUSE] = mcause;              // Cause of trap
        csr_register[MEPC] = pc;                    // Value of PC at time of trap
        csr_register[MODEP] = csr_register[MODE];   // Mode of CPU before trap
        csr_register[MODE] = 2;                     // CPU to machine mode upon trap
        csr_register[MIE] = 0;                      // Reset MIE
        csr_register[MPI] = 0;                      // Reset MPI
    }
}

void CSR::update_csr(uint32_t csr_addr, uint32_t wd)
{
    // Check if updating CSR
    if ((csr_register[MODE] == 2) & this->csr_we) {
        csr_register[csr_addr] = wd;
    }
}

uint32_t CSR::get_csr(uint32_t addr)
{
    return csr_register[addr];
}

class CSR;
#ifndef CSR_H
#define CSR_H

#include <stdint.h>

// Privilege Levels
#define USER_MODE                   0b00
#define SUPERVISOR_MODE             0b01
#define RESERVED                    0b10
#define MACHINE_MODE                0b11

// CSR Machine Mode Addresses
#define MSTATUS                     0x300
#define MISA                        0x301
#define MTVEC                       0x305
#define MIE                         0x304
#define MIP                         0x344
#define MCAUSE                      0x342
#define MEPC                        0x341
#define MSCRATCH                    0x340
#define MTVAL                       0x343
#define MVENDORID                   0xF11
#define MARCHID                     0xF12
#define MIMPID                      0xF13
#define MHARTID                     0xF14

// Memory Mapped CSR Addressed
#define MCYCLE_H                    CSR_BASE
#define MCYCLE_L                    (CSR_BASE + 4)
#define MTIMECMP_H                  (CSR_BASE + 8)
#define MTIMECMP_L                  (CSR_BASE + 12)
#define KEYBOARD                    (CSR_BASE + 16)

#define MSTATUS_MIE_MASK            3
#define MSTATUS_MPIE_MASK           7
#define MSTATUS_MPP_MASK            11

#define MIE_MEIE_MASK               11
#define MIE_MTIE_MASK               7
#define MIE_MSIE_MASK               3

#define MIP_MEIP_MASK               11
#define MIP_MTIP_MASK               7
#define MIP_MSIP_MASK               3

class Csr {
public:
    Csr(void);
    ~Csr();

    uint32_t readCsr(uint32_t address);
    void writeCsr(uint32_t address, uint32_t data);

    // CSR Bitfield operations
    void setMie(void);
    void setMpie(void);
    void setMpp(int mask);
    void setMeip(void);
    void setMtip(void);
    void setMsip(void);
    void resetMie(void);
    void resetMpie(void);
    void resetMpp(void);
    void resetMeip(void);
    void resetMtip(void);
    void resetMsip(void);
    uint32_t getMie(void);
    uint32_t getMpie(void);
    uint32_t getMpp(void);
    uint32_t getMeie(void);
    uint32_t getMtie(void);
    uint32_t getMsie(void);
    uint32_t getMeip(void);
    uint32_t getMtip(void);
    uint32_t getMsip(void);

    // CSR Standard operations
    void setMepc(uint32_t mepc);
    void setMcause(uint32_t mcause);
    uint32_t getMepc(void);
    uint32_t getMtvec(void);

private:
    // Machine Instruction Set Architecture (I)
    uint32_t misa;

    // Machine Vendor ID (no implementation)
    uint32_t mvendorid;

    // Machine Architecture ID (no implementation)
    uint32_t marchid;

    // Machine Implementation ID (no implementation)
    uint32_t mimpid;

    // Machine Hart ID
    uint32_t mhartid;

    // Machine Status
    uint32_t mstatus;

    uint32_t mtvec;
    uint32_t mie;
    uint32_t mip;
    uint32_t mcause;
    uint32_t mepc;
    uint32_t mscratch; // Maybe no implementation
    uint32_t mtval;
};

#endif // CSR_H
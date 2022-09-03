#ifndef CSR_H
#define CSR_H

#include <stdint.h>

// Privilege Levels
#define USER_MODE                   0b00
#define SUPERVISOR_MODE             0b01
#define RESERVED                    0b10
#define MACHINE_MODE                0b11

// CSR Machine Mode Addresses
#define MSTATUS     0x300
#define MISA        0x301
#define MTVEC       0x305
#define MIE         0x304
#define MIP         0x344
#define MCAUSE      0x342
#define MEPC        0x341
#define MSCRATCH    0x340
#define MTVAL       0x343
#define MVENDORID   0xF11
#define MARCHID     0xF12
#define MIMPID      0xF13
#define MHARTID     0xF14

// Memory Mapped CSR Addressed
#define MCYCLE_H    CSR_BASE
#define MCYCLE_L    (CSR_BASE + 4)
#define MTIMECMP_H  (CSR_BASE + 8)
#define MTIMECMP_L  (CSR_BASE + 12)
#define KEYBOARD    (CSR_BASE + 16)

#define MSTATUS_MIE_MASK 3
#define MSTATUS_MPIE_MASK 7
#define MSTATUS_MPP_MASK 11

#define MIE_MEIE_MASK 11
#define MIE_MTIE_MASK 7
#define MIE_MSIE_MASK 3

#define MIP_MEIP_MASK 11
#define MIP_MTIP_MASK 7
#define MIP_MSIP_MASK 3

class CSR {
public:
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

    inline void set_mie() {mstatus |= (1 << MSTATUS_MIE_MASK);}
    inline void set_mpie() {mstatus |= (1 << MSTATUS_MPIE_MASK);}
    inline void set_mpp(int mask) {mstatus |= ((mask & 0b11) << MSTATUS_MPP_MASK);}
    inline void set_meip() {mip |= (1 << MIP_MEIP_MASK);}
    inline void set_mtip() {mip |= (1 << MIP_MTIP_MASK);}
    inline void set_msip() {mip |= (1 << MIP_MSIP_MASK);}

    inline void reset_mie() {mstatus &= ~(1 << MSTATUS_MIE_MASK);}
    inline void reset_mpie() {mstatus &= ~(1 << MSTATUS_MPIE_MASK);}
    inline void reset_mpp() {mstatus &= ~(0b11 << MSTATUS_MPP_MASK);}
    inline void reset_meip() {mip &= ~(1 << MIP_MEIP_MASK);}
    inline void reset_mtip() {mip &= ~(1 << MIP_MTIP_MASK);}
    inline void reset_msip() {mip &= ~(1 << MIP_MSIP_MASK);}

    inline uint32_t get_mie() {return ((mstatus >> MSTATUS_MIE_MASK) & 0b1);}
    inline uint32_t get_mpie() {return ((mstatus >> MSTATUS_MPIE_MASK) & 0b1);}
    inline uint32_t get_mpp() {return ((mstatus >> MSTATUS_MPP_MASK) & 0b11);}
    inline uint32_t get_meie() {return ((mie >> MIE_MEIE_MASK) &0b1);}
    inline uint32_t get_mtie() {return ((mie >> MIE_MTIE_MASK) &0b1);}
    inline uint32_t get_msie() {return ((mie >> MIE_MSIE_MASK) &0b1);}
    inline uint32_t get_meip() {return ((mip >> MIP_MEIP_MASK) &0b1);}
    inline uint32_t get_mtip() {return ((mip >> MIP_MTIP_MASK) &0b1);}
    inline uint32_t get_msip() {return ((mip >> MIP_MSIP_MASK) &0b1);}

    CSR();
    uint32_t read_csr(uint32_t);
    void write_csr(uint32_t, uint32_t);
};

#endif // CSR_H
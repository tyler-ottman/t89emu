#ifndef CSR_H
#define CSR_H

#include <stdint.h>
#include <string>
#include <unordered_map>

// Privilege Levels
#define USER_MODE                   0b00
#define SUPERVISOR_MODE             0b01
#define RESERVED                    0b10
#define MACHINE_MODE                0b11

// CSR Machine Mode Addresses
enum ControlStateRegisters {
    CSR_MSTATUS = 0x300,
    CSR_MISA = 0x301,
    CSR_MTVEC = 0x305,
    CSR_MIE = 0x304,
    CSR_MIP = 0x344,
    CSR_MCAUSE = 0x342,
    CSR_MEPC = 0x341,
    CSR_MSCRATCH = 0x340,
    CSR_MTVAL = 0x343,
    CSR_MVENDORID = 0xF11,
    CSR_MARCHID = 0xF12,
    CSR_MIMPID = 0xF13,
    CSR_MHARTID = 0xF14
};

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
    std::unordered_map<uint32_t, uint32_t> registers;
};

#endif // CSR_H
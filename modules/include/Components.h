#include <stdint.h>
#include <unordered_map>
#include <vector>

// opcodes
#define LUI   0b0110111
#define AUIPC 0b0010111
#define JAL   0b1101111
#define JALR  0b1100111
#define BTYPE 0b1100011
#define LOAD  0b0000011
#define STORE 0b0100011 
#define ITYPE 0b0010011
#define RTYPE 0b0110011
#define PRIV  0b1110011

#define ECALL_IMM   0b000000000000
#define MRET_IMM    0b001100000010
#define URET_IMM    0b000000000010

#ifndef ALU_H
#define ALU_H

#define ADD  0
#define SUB  1
#define OR   2
#define AND  3
#define XOR  4
#define SRL  5
#define SRA  6
#define SLL  7
#define SLT  8
#define SLTU 9

class ALU
{
public:
    uint32_t execute(uint32_t, uint32_t, int);
};

#endif // ALU_H

#ifndef ALUCONTROLUNIT_H
#define ALUCONTROLUNIT_H

class ALUControlUnit
{
private:
    std::vector<int> i_operations;
    std::vector<int> r_operations;

public:
    ALUControlUnit();
    int getALUoperation(int, int, int);
};

#endif // ALUCONTROLUNIT_H

#ifndef PROGRAMCOUNTER_H
#define PROGRAMCOUNTER_H

class ProgramCounter
{
public:
    uint32_t PC = 0;
    ProgramCounter();
    void setPC(uint32_t);
    uint32_t getPC();
};

#endif // PROGRAMCOUNTER_H

#ifndef REGISTERFILE_H
#define REGISTERFILE_H

class RegisterFile
{
private:
    uint32_t* registers;

public:
    RegisterFile();
    uint32_t read(int);
    void write(uint32_t, int);
};

#endif // REGISTERFILE_H

#ifndef IMMEDIATEGENERATOR_H
#define IMMEDIATEGENERATOR_H

class ImmediateGenerator
{
public:
    uint32_t getImmediate(uint32_t);
};

#endif // IMMEDIATEGENERATOR_H

#ifndef NEXTPC_H
#define NEXTPC_H

class NextPC
{
private:
    uint32_t nextPC;
    int branch_alu(uint32_t, uint32_t, uint32_t);
public:
    NextPC();
    uint32_t calculateNextPC(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
};

#endif // NEXTPC_H

#ifndef MEMCONTROLUNIT_H
#define MEMCONTROLUNIT_H

class MemControlUnit
{
public:
    int get_mem_size(uint32_t);
};

#endif // MEMORY_H

#ifndef MEMORYDEVICE_H
#define MEMORYDEVICE_H

class MemoryDevice {
public:
    //MemoryDevice(uint32_t, uint32_t);
    MemoryDevice();
    virtual ~MemoryDevice();
    virtual uint8_t* get_address(uint32_t);
    virtual uint32_t read(uint32_t, uint32_t) = 0;
    virtual void write(uint32_t, uint32_t, uint32_t) = 0;
    uint32_t baseAddress;
    uint32_t deviceSize;
    uint8_t* mem;
};

#endif

#ifndef CSR_MEMORYDEVICE_H
#define CSR_MEMORYDEVICE_H

class CSRMemoryDevice : public MemoryDevice {
public:
    CSRMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // CSR_MEMORYDEVICE_H

#ifndef ROM_MEMORYDEVICE_H
#define ROM_MEMORYDEVICE_H

class ROMMemoryDevice : public MemoryDevice {
public:
    ROMMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // ROM_MEMORYDEVICE_H

#ifndef RAM_MEMORYDEVICE_H
#define RAM_MEMORYDEVICE_H

class RAMMemoryDevice : public MemoryDevice {
public:
    RAMMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // RAM_MEMORYDEVICE_H

#ifndef VIDEO_MEMORYDEVICE_H
#define VIDEO_MEMORYDEVICE_H

class VideoMemoryDevice : public MemoryDevice {
public:
    VideoMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // VIDEO_MEMORYDEVICE_H

#ifndef BUS_H
#define BUS_H

#define BYTE 1
#define HALFWORD 2
#define WORD 4

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 288

// Some memory devices are at fixed addresses

// CSR Memory-Mapped IO Device
#define CSR_BASE 0x30000000
#define CSR_SIZE 0x14
#define CSR_END (CSR_BASE + CSR_SIZE)

// Video Memory Device
#define VIDEO_BASE 0x20000000
#define VIDEO_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT * WORD)
#define VIDEO_END (VIDEO_BASE + VIDEO_SIZE)

// Future: UART
// Future: PLIC
class Bus {
    
public:
    Bus(uint32_t, uint32_t, uint32_t, uint32_t);
    ~Bus();
    void write(uint32_t, uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    ROMMemoryDevice* rom_device;
    RAMMemoryDevice* ram_device;
    VideoMemoryDevice* video_device;
    CSRMemoryDevice* csr_device;
    uint32_t rom_base;
    uint32_t rom_end;
    uint32_t ram_base;
    uint32_t ram_end;
};

#endif // BUS_H

#ifndef TRAP_H
#define TRAP_H

class Trap {
public:

};

#endif // TRAP_H

#ifndef CSR_H
#define CSR_H

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
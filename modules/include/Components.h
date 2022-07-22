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
#define ECALL 0b1110011

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
    uint32_t calculateNextPC(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
};

#endif // NEXTPC_H

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
    uint32_t mcycles;

    inline void set_mie(int mask) {mstatus = mstatus | ((mask&0b1) << 3);}
    inline void set_mpie(int mask) {mstatus = mstatus | ((mask&0b1) << 7);}
    inline void set_mpp(int mask) {mstatus = mstatus | ((mask&0b11) << 11);}

    inline uint32_t get_mie() {return ((mstatus >> 3) & 0b1);}
    inline uint32_t get_mpie() {return ((mstatus >> 7) & 0b1);}
    inline uint32_t get_mpp() {return ((mstatus >> 11) & 0b11);}

    CSR();
    uint32_t read_csr(uint32_t);
    void write_csr(uint32_t, uint32_t);
};

#endif // CSR_H

#ifndef MEMORY_H
#define MEMORY_H

#define BYTE 1
#define HALFWORD 2
#define WORD 4

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 288

#define INSTRUCTION_MEMORY_START    0x00000000 // Beginning of Instruction Memory
#define DATA_MEMORY_START           0x10000000 // Beginning of Data Memory
#define VIDEO_MEMORY_START          0x20000000 // Beginning of Video Memory
#define CSR_MEMORY_START            0x30000000 // Beginning of CSR Memory Mapped Registers

#define INSTRUCTION_MEMORY_SIZE (WORD * 32768) // 128 KB
#define DATA_MEMORY_SIZE (WORD * 262144) // 1 MB
#define VIDEO_MEMORY_SIZE (WORD * SCREEN_WIDTH * SCREEN_HEIGHT) // About 590 KB

class Memory
{
public:
    uint32_t instruction_memory[INSTRUCTION_MEMORY_SIZE] = { 0 };      // 128 KB Instruction Memory
    uint32_t data_memory[DATA_MEMORY_SIZE] = { 0 };
    uint32_t video_memory[VIDEO_MEMORY_SIZE] = { 0 }; // 512x288 Video Memory
    uint32_t csr_memory[4] = { 0 };
    void write(uint32_t, uint32_t, int);
    uint32_t read(uint32_t, int);
};

#endif // MEMORY_H

#ifndef MEMCONTROLUNIT_H
#define MEMCONTROLUNIT_H

class MemControlUnit
{
public:
    int get_mem_size(uint32_t);
};

#endif // MEMORY_H
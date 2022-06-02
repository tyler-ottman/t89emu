#include <stdint.h>
#include <unordered_map>
#include <vector>

#define VRAM_START 0x40000000    // Beginning of VRAM
#define WIDTH 50
#define HEIGHT 50
#define VRAM_LEN (4 * WIDTH * HEIGHT) // WIDTHxHEIGHT pixels each 4 bytes

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

#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

class ControlUnit
{
private:
    std::unordered_map<int, uint32_t> signals;
public:
    ControlUnit();
    std::vector<uint32_t> getControlLines(int, int, int, int);
};

#endif // CONTROLUNIT_H

#ifndef PROGRAMCOUNTER_H
#define PROGRAMCOUNTER_H

class ProgramCounter
{
private:
    uint32_t PC;

public:
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

template <typename T>
class NextPC
{
private:
    T nextPC;
    int branch_alu(T, T, T);

public:
    NextPC();
    void setCurrentPC(T);
    void calculateNextPC(T, T, T, T, T, T, T);
    T getNextPC();
};

#endif // NEXTPC_H

#ifndef CSR_H
#define CSR_H

class CSR
{

private:
    uint32_t* csr_register;
    uint32_t csr_we;
    uint32_t trap_taken;

public:
    CSR();
    void set_control_lines(uint32_t, uint32_t, uint32_t, uint32_t);
    void update_csr(uint32_t, uint32_t);
    uint32_t get_csr(uint32_t);
};

#endif // CSR_H

#ifndef MEMORY_H
#define MEMORY_H

#define BYTE 1
#define HALFWORD 2
#define WORD 4

class Memory
{
private:
    std::unordered_map<uint32_t, uint32_t> dram;
    uint32_t changed_pixel;

public:
    void write(uint32_t, uint32_t, int);
    uint32_t read(uint32_t, int);
    uint32_t get_changed_pixel();
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
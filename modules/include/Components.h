#include <stdint.h>
#include <unordered_map>
#include <vector>

#define VRAM_START 0x40000000    // Beginning of VRAM
#define WIDTH 50
#define HEIGHT 50
#define VRAM_LEN (4 * WIDTH * HEIGHT) // WIDTHxHEIGHT pixels each 4 bytes

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

#ifndef ALU_H
#define ALU_H

class ALU
{
public:
    ALU();
    uint32_t exec(uint32_t, uint32_t, int);

private:
    int size_of_operand;
    uint32_t MSB;
    uint32_t isPositiveA;
    uint32_t isPositiveB;
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
    int opcode;
    int RegWrite;
    int ALUSrc;
    int MemtoReg;
    int MemRead;
    int MemWrite;
    int PC_Select;
    int CSR_enable;
    int trap;
    uint32_t mcause;
    int ALUop;

    int getInstructionType();

public:
    ControlUnit();
    void setControlLines(int, int, int, int);
    int get_RegWrite();
    int get_ALUSrc();
    int get_MemtoReg();
    int get_MemRead();
    int get_MemWrite();
    int get_PC_Select();
    int get_CSR_enable();
    int get_trap();
    int get_mcause();
    int get_ALUop();
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

template <typename T>
class RegisterFile
{

private:
    T *registers;
    int rs1;
    int rs2;
    int rd;
    int Reg_Write;

public:
    RegisterFile();
    void set_control_lines(int, int, int);
    T read_rs1();
    T read_rs2();
    T read_rd();
    void write(T, int);
};

#endif // REGISTERFILE_H

#ifndef IMMEDIATEGENERATOR_H
#define IMMEDIATEGENERATOR_H

template <typename T>
class ImmediateGenerator
{
private:
    T getInstrType();
    std::vector<T> opcodes;
    T immediate;
    T funct3;
    T opcode;
public:
    // Support 64-bit in the future
    ImmediateGenerator();
    T getImmediate(T);
};

#endif // IMMEDIATEGENERATOR_H

#ifndef NEXTPC_H
#define NEXTPC_H

template <typename T>
class NextPC
{
private:
    T nextPC;
    int jal_jalr_branch(T);
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

class Memory
{
private:
    uint32_t data_out;
    uint32_t instr_out;
    std::unordered_map<uint32_t, uint32_t> dram;
    int MemReadData;
    int MemWriteData;
    int size;
    int IO_WR_enable;
    uint32_t changed_pixel;

public:
    Memory();
    void set_control_signals(int, int, int, int);
    void write_data(uint32_t addr, uint32_t data);
    void write_io(uint32_t, uint32_t);
    uint32_t read_data(uint32_t addr);
    uint32_t get_changed_pixel();
};

#endif // MEMORY_H
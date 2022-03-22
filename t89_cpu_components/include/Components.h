#ifndef ALU_H
#define ALU_H
#include <stdint.h>
#include <unordered_map>

template <typename T>
class ALU
{
public:
    ALU();
    void printOperands(void);
    T exec(T, T, int);

private:
    T A;
    T B;
    T *A_bits;
    T *B_bits;
    int size_of_operand;
    void setOperands(T A, T B);
    T getNum(T *arr);
    T *fullAdder(T a, T b, T carry);
    T invertNum(T num);
    void bitShiftRight(T isArithmetic);
    void bitShiftLeft();
    T add();
    T sub();
    T _or();
    T _and();
    T _xor();
    T srl();
    T sra();
    T sll();
    T slt();
    T sltu();
};

#endif // ALU_H

#ifndef ALUCONTROLUNIT_H
#define ALUCONTROLUNIT_H

class ALUControlUnit
{
private:
    int i_type;
    int r_type;
    int get_i_type(int, int);
    int get_r_type(int, int);

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
    void write(T, int);
};

#endif // REGISTERFILE_H

#ifndef IMMEDIATEGENERATOR_H
#define IMMEDIATEGENERATOR_H

template <typename T>
class ImmediateGenerator
{
private:
    T instruction;
    int instr_opcode;
    int getInstrType();
    T *getBinaryArray(T);
    T getNum(T *, int);

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

public:
    Memory();
    void set_control_signals(int, int, int, int);
    void write_data(uint32_t addr, uint32_t data);
    void write_io(uint32_t, uint32_t);
    uint32_t read_data(uint32_t addr);
};

#endif // MEMORY_H
#include <iostream>
#include <vector>
#include <deque>

#include <gtest/gtest.h>
#include "Components.h"

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

// ALU Module Operations Test
TEST(ALU, ALU_test)  {
    ALU alu;
    uint32_t res;
    
    /*****************************************
     ************ ALU OPERATION **************
     ************  1ST OPERAND  **************
     ************  2ND OPERAND  **************
     ****************************************/
    std::vector<std::vector<uint32_t>> arguments = {
        // ADD
        {0x0000000f, 0x0000f3e9, 0x000000ff, 0x000000ff},
        {0x000000ff, 0xffffffff, 0x00000001, 0x00000000},
        // SUB
        {0x0000001f, 0xffffffff, 0x00000000},
        {0x0000000f, 0x0000000f, 0x0000000f},
        // OR
        {0x00000006, 0x8000000f},
        {0x00000009, 0xf000000f},
        // AND
        {0x00008000, 0x00000003},
        {0x0000f001, 0x00000006},
        // XOR
        {0x00008000, 0x0000ffff},
        {0x00007fff, 0x0000ffff},
        // SRL
        {0x00000008, 0x80000000, 0xffffffff},
        {0x00000001, 0x0000001f, 0x00000028},
        // SRA
        {0x80000000, 0x80000000, 0x00000008},
        {0x00000001, 0x00000003, 0x00000002},
        // SLL
        {0x00000001, 0x00000001, 0x00000001},
        {0x00000001, 0x0000001f, 0x00000020},
        // SLT
        {0x00000001, 0xffffffff, 0x80000001, 0x00000004, 0x80000001, 0xffffffff, 0x0000000a},
        {0x00000002, 0x00000005, 0x00000001, 0x00000004, 0xffffffff, 0x80000001, 0x00000005},
        // SLTU
        {0x00000003, 0xfffffffe, 0xffffffff, 0x0000000f, 0x00000fff, 0xfffffff0, 0x80000000},
        {0x00000005, 0xffffffff, 0x80000000, 0xffffffff, 0x80000000, 0x00000fac, 0x00000001}
    };

    // Results that can't be calculated with operators must be specified manually (SRA, SLL, SLT, SLTU)
    std::deque<uint32_t> r_sra  = {0xc0000000, 0xf0000000, 0x00000002};
    std::deque<uint32_t> r_slt  = {0x00000001, 0x00000001, 0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000000};
    std::deque<uint32_t> r_sltu = {0x00000001, 0x00000001, 0x00000000, 0x00000001, 0x00000001, 0x00000000, 0x00000000};

    // Error check
    for (size_t i = 0; i < arguments.size(); i += 2) {
        size_t arg_size = arguments.at(i).size();
        int ALU_op = i / 2;
        // Special operations result size check
        if (((ALU_op == (size_t)SRA)  && (arg_size != r_sra.size()) ) ||
            ((ALU_op == (size_t)SLT)  && (arg_size != r_slt.size()) ) ||
            ((ALU_op == (size_t)SLTU) && (arg_size != r_sltu.size()))) {
                std::cerr << "Special results: Invalid Test Dimensions" << std::endl;
                exit(EXIT_FAILURE);
        }
        // Verify operand dimensions
        if (arguments.at(i).size() != arguments.at(i + 1).size()) {
            std::cerr << "Operands: Invalid Test Dimensions" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Test ALU operations
    for (size_t i = 0; i < arguments.size(); i += 2) {
        int ALU_op = i / 2;
        for (size_t j = 0; j < arguments.at(i).size(); j++) {
            switch (ALU_op)
            {
            case ADD:
                res = arguments.at(i).at(j) + arguments.at(i + 1).at(j);
                break;
            case SUB:
                res = arguments.at(i).at(j) - arguments.at(i + 1).at(j);
                break;
            case OR:
                res = arguments.at(i).at(j) | arguments.at(i + 1).at(j);
                break;
            case AND:
                res = arguments.at(i).at(j) & arguments.at(i + 1).at(j);
                break;
            case XOR:
                res = arguments.at(i).at(j) ^ arguments.at(i + 1).at(j);
                break;
            case SRL:
                res = arguments.at(i).at(j) >> arguments.at(i + 1).at(j);
                break;
            case SRA:
                res = r_sra.at(0); 
                r_sra.pop_front();
                break;
            case SLL:
                res = arguments.at(i).at(j) << arguments.at(i + 1).at(j);
                break;
            case SLT:
                res = r_slt.at(0); 
                r_slt.pop_front();
                break;
            case SLTU:
                res = r_sltu.at(0);
                r_sltu.pop_front();
                break;
            default:
                std::cout << "Unexpected error" << std::endl;
                exit(EXIT_FAILURE);
            }
            EXPECT_EQ(res, alu.exec(arguments.at(i).at(j), arguments.at(i + 1).at(j), ALU_op));   
        }
    }
}

// Test ALU Control Unit
TEST(ALUControlUnit, Misc_Instr) {
    ALUControlUnit misc;
    int ALUop[6] = {0, 1, 2, 3, 5, 6}; // lui, add, jal, jalr, loads, stores
    int funct7 = 0;
    int funct3 = 0;
    for (int x : ALUop) {
        EXPECT_EQ(0, misc.getALUoperation(x, funct7, funct3));
    }
}

TEST(ALUControlUnit, I_type) {
    ALUControlUnit i_type;
    int ALUop = 7;
    // addi, slti, sltiu, xori, ori, andi, slli
    int funct3[7] = {0b000, 0b010, 0b011, 0b100, 0b110, 0b111, 0b001};
    int funct7 = 0;
    int index = 0;
    int expected[7] = {0, 8, 9, 4, 2, 3, 7};
    int ALU_operation;
    for (int x : funct3) {
        ALU_operation = i_type.getALUoperation(ALUop, funct7, x);
        EXPECT_EQ(expected[index++], ALU_operation);
    }

    // srli/srai
    int funct3_2 = 0b101;
    int funct7_2[2] = {0b0000000, 0b0100000};
    EXPECT_EQ(5, i_type.getALUoperation(ALUop, funct7_2[0], funct3_2));
    EXPECT_EQ(6, i_type.getALUoperation(ALUop, funct7_2[1], funct3_2));
}

TEST(ALUControlUnit, R_type) {
    ALUControlUnit alucu;

    int ALUop = 7;
    // addi, slti, sltiu, xori, ori, andi, slli
    std::vector<int> funct3 = {0b000, 0b010, 0b011, 0b100, 0b110, 0b111, 0b001};
    std::vector<int> funct7 = {0};
    int index = 0;
    std::vector<int> expected = {0, 8, 9, 4, 2, 3, 7}; // Expected ALU operation code
    for (int x : funct3) {
        int ALU_operation = alucu.getALUoperation(ALUop, funct7.at(0), x);
        EXPECT_EQ(expected.at(index++), ALU_operation);
    }

    // srli/srai
    funct7 = {0b0000000, 0b0100000};
    EXPECT_EQ(5, alucu.getALUoperation(ALUop, funct7.at(0), 0b101));
    EXPECT_EQ(6, alucu.getALUoperation(ALUop, funct7.at(1), 0b101));

    // Branch
    EXPECT_EQ(1, alucu.getALUoperation(4, 0, 0));

    // add / sub R-type
    funct7 = {0b0000000, 0b0100000};
    ALUop = 8;
    expected = {0, 1};
    index = 0;
    for (int x : funct7) {
        EXPECT_EQ(expected[index++], alucu.getALUoperation(ALUop, x, 0b000));
    }

    // SRL / SRA R-type
    funct7 = {0b0000000, 0b0100000};
    expected = {5, 6};
    index = 0;
    for (int x : funct7) {
        EXPECT_EQ(expected[index++], alucu.getALUoperation(ALUop, x, 0b101));
    }

    // sll, slt, sltu, xor, or, and
    funct3 = {0b001, 0b010, 0b011, 0b100, 0b110, 0b111};
    expected = {7, 8, 9, 4, 2, 3};
    index = 0;
    for (int x : funct3) {
        EXPECT_EQ(expected[index++], alucu.getALUoperation(ALUop, 0, x));
    }
}

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

// Control Unit
TEST(ControlUnit, opcodes) {
    ControlUnit ctrl;

    int interrupt = 0;
    int funct3 = 0;
    int supervisor_mode = 0;

    // lui
    std::vector<uint32_t> signals_bus = ctrl.getControlLines(LUI, interrupt, funct3, supervisor_mode);
    uint32_t RegWrite  = (signals_bus[0] >> 10) & 0b11;
    uint32_t ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    uint32_t MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    uint32_t MemRead   = (signals_bus[0] >>  4) & 0b11;
    uint32_t MemWrite  = (signals_bus[0] >>  2) & 0b11;
    uint32_t PC_select = (signals_bus[0]      ) & 0b11;
    uint32_t trap_taken = signals_bus[1];
    uint32_t mcause = signals_bus[2];
    uint32_t CSR_en = signals_bus[3];
    EXPECT_EQ(1, RegWrite);
    EXPECT_EQ(2, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(2, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // auipc
    signals_bus = ctrl.getControlLines(AUIPC, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(1, RegWrite);
    EXPECT_EQ(2, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(1, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // jal
    signals_bus = ctrl.getControlLines(JAL, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(1, RegWrite);
    EXPECT_EQ(0, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(1, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // jalr
    signals_bus = ctrl.getControlLines(JALR, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(1, RegWrite);
    EXPECT_EQ(0, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(1, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // b-type
    signals_bus = ctrl.getControlLines(BTYPE, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(0, RegWrite);
    EXPECT_EQ(1, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // Load
    signals_bus = ctrl.getControlLines(LOAD, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(1, RegWrite);
    EXPECT_EQ(2, ALUSrc);
    EXPECT_EQ(1, MemtoReg);
    EXPECT_EQ(1, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // store
    signals_bus = ctrl.getControlLines(STORE, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(0, RegWrite);
    EXPECT_EQ(2, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(1, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // itype
    signals_bus = ctrl.getControlLines(ITYPE, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(1, RegWrite);
    EXPECT_EQ(2, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // r-type
    signals_bus = ctrl.getControlLines(RTYPE, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(1, RegWrite);
    EXPECT_EQ(1, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // ecall
    supervisor_mode = 1;
    funct3 = 0;
    signals_bus = ctrl.getControlLines(ECALL, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(0, RegWrite);
    EXPECT_EQ(0, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(1, trap_taken);
    EXPECT_EQ(0x00000008, mcause);
    EXPECT_EQ(0, CSR_en);

    // Machine Hardware Interrupt
    interrupt = 2;
    signals_bus = ctrl.getControlLines(AUIPC, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(0, RegWrite);
    EXPECT_EQ(0, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(1, trap_taken);
    EXPECT_EQ(0x8000000b, mcause);
    EXPECT_EQ(0, CSR_en);

    // Machine Timer Interrupt
    interrupt = 1;
    signals_bus = ctrl.getControlLines(AUIPC, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(0, RegWrite);
    EXPECT_EQ(0, ALUSrc);
    EXPECT_EQ(0, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(1, trap_taken);
    EXPECT_EQ(0x80000007, mcause);
    EXPECT_EQ(0, CSR_en);

    // Write to CSR
    interrupt = 0;
    funct3 = 0b001;
    signals_bus = ctrl.getControlLines(ECALL, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(0, RegWrite);
    EXPECT_EQ(0, ALUSrc);
    EXPECT_EQ(2, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(1, CSR_en);

    // Write to CSR in user mode
    supervisor_mode = 0;
    signals_bus = ctrl.getControlLines(ECALL, interrupt, funct3, supervisor_mode);
    RegWrite  = (signals_bus[0] >> 10) & 0b11;
    ALUSrc    = (signals_bus[0] >>  8) & 0b11;
    MemtoReg  = (signals_bus[0] >>  6) & 0b11;
    MemRead   = (signals_bus[0] >>  4) & 0b11;
    MemWrite  = (signals_bus[0] >>  2) & 0b11;
    PC_select = (signals_bus[0]      ) & 0b11;
    trap_taken = signals_bus[1];
    mcause = signals_bus[2];
    CSR_en = signals_bus[3];
    EXPECT_EQ(0, RegWrite);
    EXPECT_EQ(0, ALUSrc);
    EXPECT_EQ(2, MemtoReg);
    EXPECT_EQ(0, MemRead);
    EXPECT_EQ(0, MemWrite);
    EXPECT_EQ(0, PC_select);
    EXPECT_EQ(0, trap_taken);
    EXPECT_EQ(0, mcause);
    EXPECT_EQ(0, CSR_en);

    // lui, auipc, jal, jalr, b-type, load, store, i-type, r-type, ecall
    // int opcodes[10] =      {LUI,       AUIPC,     JAL,       JALR,      BTYPE,     LOAD,      STORE,     ITYPE,     RTYPE,     ECALL    };
    // int controlLines[10] = {0b1100000, 0b1100001, 0b1000001, 0b1000001, 0b0010000, 0b1101100, 0b0100010, 0b1100000, 0b1010000, 0b0000000};
    // int index = 0;
    // for (int x : opcodes) {
    //     int regWrite = controlLines[index] >> 6;
    //     int ALUsrc = (controlLines[index] >> 4) & 3;
    //     int MemToReg = (controlLines[index] >> 3) & 1;
    //     int MemRead = (controlLines[index] >> 2) & 1;
    //     int MemWrite = (controlLines[index] >> 1) & 1;
    //     int PC_select = (controlLines[index] & 1);
    //     int CSR_enable = 0;
    //     index++;
    //     signals.setControlLines(x, 0, 0, 0);
    //     EXPECT_EQ(regWrite, signals.get_RegWrite());
    //     EXPECT_EQ(ALUsrc, signals.get_ALUSrc());
    //     EXPECT_EQ(MemToReg, signals.get_MemtoReg());
    //     EXPECT_EQ(MemRead, signals.get_MemRead());
    //     EXPECT_EQ(MemWrite, signals.get_MemWrite());
    //     EXPECT_EQ(PC_select, signals.get_PC_Select());
    //     EXPECT_EQ(CSR_enable, signals.get_CSR_enable());
    // }

    // // setControlLines(int opcode, int csr_interrupt, int funct3, int supervisor_mode)
    // signals.setControlLines(0b1110011, 0, 0b001, 1); // csrrw
    // EXPECT_EQ(1, signals.get_CSR_enable());

    // signals.setControlLines(0b1110011, 0, 0b010, 0); // csrrs
    // EXPECT_EQ(0, signals.get_CSR_enable());

    // signals.setControlLines(0b0110011, 1, 0b000, 0); // machine timer interrupt
    // EXPECT_EQ(0x80000007, signals.get_mcause());
    // EXPECT_EQ(1, signals.get_trap());

    // signals.setControlLines(0b0110011, 2, 0b000, 0); // machine external interrupt
    // EXPECT_EQ(0x8000000b, signals.get_mcause());
    // EXPECT_EQ(1, signals.get_trap());

    // signals.setControlLines(0b1110011, 0, 0b000, 0); // user mode ecall
    // EXPECT_EQ(1, signals.get_trap());
    // EXPECT_EQ(0x8, signals.get_mcause());
}

// Register File
TEST(RegisterFile, read_write) {
    RegisterFile<uint32_t> rf;
    int rs1 = 10;
    int rs2 = 3;
    uint32_t data = 0xff;
    rf.write(data, rs1);
    rf.set_control_lines(rs1, rs2, 1);
    EXPECT_EQ(data, rf.read_rs1());
    data = 0x12;
    rf.write(data, rs2);
    EXPECT_EQ(data, rf.read_rs2());
}

TEST(ProgramCounter, setPC) {
    ProgramCounter pc;
    pc.setPC(0xfff);
    EXPECT_EQ(0xfff, pc.getPC());
}

TEST(ImmediateGenerator, immediates) {
    ImmediateGenerator<uint32_t> imm;
    uint32_t instruction;

    // Upper Immediate Instructions (jal here)
    std::vector<uint32_t> opcode =    {LUI,     AUIPC};
    std::vector<uint32_t> immediate = {0xabcdf, 0xfdcba};
    for (size_t i = 0; i < opcode.size(); i++) {
        if (opcode[i] == AUIPC || opcode[i] == LUI) {
            immediate[i] = immediate[i] << 12;
            instruction = immediate[i] | opcode[i];
            EXPECT_EQ(immediate[i], imm.getImmediate(instruction));
        }
    }
    // Stores (B-Type here)
    opcode =                     {STORE,     STORE};
    std::vector<int> left_imm  = {0b0111111, 0b1111111};
    std::vector<int> right_imm = {0b11111,   0b11111};
    for (size_t i = 0; i < opcode.size(); i++) {
        instruction = (left_imm[i] << 25) + (right_imm[i] << 7) + opcode[i];
        immediate[i] = ((left_imm[i] << 5) + (right_imm[i]));
        if ((left_imm[i] >> 6)) {
            immediate[i] |= 0xfffff000;
        }
        EXPECT_EQ(immediate[i], imm.getImmediate(instruction));
    }

    // Jalr, Load, I-type
    opcode    = {JALR,  LOAD,  ITYPE};
    immediate = {0xfff, 0xaaa, 0xcba};
    for (size_t i = 0; i < opcode.size(); i++) {
        instruction = (immediate[i] << 20) | opcode[i];
        if (instruction >> 31) {
            immediate[i] |= 0xfffff000;
        }
        std::cout << opcode[i] << std::endl;
        EXPECT_EQ(immediate[i], imm.getImmediate(instruction));
    }

    // ecall
    instruction = (0x001 << 20) | ECALL;
    EXPECT_EQ(0x001, imm.getImmediate(instruction));
}

TEST(CSR, csr_test) {
    CSR csr;
    // csr_we, trap_taken, mcause, pc
    csr.set_control_lines(1, 0, 0, 0); // csrrw
    csr.update_csr(0x000, 1);
    EXPECT_EQ(1, csr.get_csr(0x000));

    csr.set_control_lines(0, 0, 0, 0); // csrrs
    csr.update_csr(0x000, 0);
    EXPECT_EQ(1, csr.get_csr(0x000));

    // Trap Taken (time interrupt when pc at 0x0000ffff)
    uint32_t pc = 0x0000ffff;
    uint32_t mcause = 0x80000007;
    csr.set_control_lines(0, 1, mcause, pc);
    EXPECT_EQ(mcause, csr.get_csr(0x004));
    EXPECT_EQ(pc, csr.get_csr(0x002));
    EXPECT_EQ(0, csr.get_csr(0x000));
    EXPECT_EQ(0, csr.get_csr(0x001));
}

#define WRITE 0
#define WRITE_IO 1
#define READ 2

TEST(Memory, read_write_test) {
    Memory dram;
    // int MemReadData, int MemWriteData, int size, int IO_WR_enable

    std::vector<uint32_t> address = {0x0000ffff, 0x0000ffff, 0x000000ff, 0x000000ff};
    std::vector<uint32_t> data    = {0x00000020, 0,          0x0000000a, 0};
    std::vector<uint32_t> mem_op  = {WRITE_IO,   READ,       WRITE,      READ};
    int previously_written_value;
    for (size_t i = 0; i < mem_op.size(); i++) {
        switch(mem_op[i]) {
            case WRITE_IO:
                dram.set_control_signals(0, 1, 4, 1);
                dram.write_io(address[i], data[i]);
                previously_written_value = data[i];
                break;
            case WRITE:
                dram.set_control_signals(0, 1, 4, 0);
                dram.write_data(address[i], data[i]);
                previously_written_value = data[i];
                break;
            case READ:
                dram.set_control_signals(1, 0, 4, 0);
                EXPECT_EQ(previously_written_value, dram.read_data(address[i]));
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
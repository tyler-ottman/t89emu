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
    ALU<uint32_t> alu;
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

TEST(ALUControlUnit, Branch) {
    ALUControlUnit branch;
    int ALUop = 4;
    int funct7 = 0;
    int funct3 = 0;
    EXPECT_EQ(1, branch.getALUoperation(ALUop, funct7, funct3));
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
        // std::cout << "Here is funct3[" << index << "] = " << x << std::endl;
        // std::cout << "We expect " << expected[index] << std::endl;
        ALU_operation = i_type.getALUoperation(ALUop, funct7, x);
        EXPECT_EQ(expected[index], ALU_operation);
        // std::cout << "What we got: " << ALU_operation << std::endl;
        index++;
    }

    // srli/srai
    int funct3_2 = 0b101;
    int funct7_2[2] = {0b0000000, 0b0100000};
    EXPECT_EQ(5, i_type.getALUoperation(ALUop, funct7_2[0], funct3_2));
    EXPECT_EQ(6, i_type.getALUoperation(ALUop, funct7_2[1], funct3_2));
}

TEST(ALUControlUnit, R_type) {
    // add / sub R-type
    ALUControlUnit r_type;
    int funct3_add_sub = 0b000;
    int funct7_add_sub[2] = {0b0000000, 0b0100000};
    int ALUop = 8;
    int expected_add_sub[2] = {0, 1};
    int index = 0;
    for (int x : funct7_add_sub) {
        EXPECT_EQ(expected_add_sub[index++], r_type.getALUoperation(ALUop, x, funct3_add_sub));
    }

    // SRL / SRA R-type
    int funct3_srl_sra = 0b101;
    int funct7_srl_sra[2] = {0b0000000, 0b0100000};
    int expected_srl_sra[2] = {5, 6};
    index = 0;
    for (int x : funct7_srl_sra) {
        EXPECT_EQ(expected_srl_sra[index++], r_type.getALUoperation(ALUop, x, funct3_srl_sra));
    }

    // All others
    // sll, slt, sltu, xor, or, and
    int funct3[6] = {0b001, 0b010, 0b011, 0b100, 0b110, 0b111};
    int funct7 = 0;
    int expected[6] = {7, 8, 9, 4, 2, 3};
    index = 0;
    for (int x : funct3) {
        EXPECT_EQ(expected[index++], r_type.getALUoperation(ALUop, funct7, x));
    }
}

// Control Unit
TEST(ControlUnit, opcodes) {
    ControlUnit signals;
    // lui, auipc, jal, jalr, b-type, load, store, i-type, r-type, ecall
    int opcodes[10] =       {0b0110111, 0b0010111, 0b1101111, 0b1100111, 0b1100011, 0b0000011, 0b0100011, 0b0010011, 0b0110011, 0b1110011};
    int controlLines[10] =  {0b1100000, 0b1100001, 0b1000001, 0b1000001, 0b0010000, 0b1101100, 0b0100010, 0b1100000, 0b1010000, 0b0000000};
    int index = 0;
    for (int x : opcodes) {
        int regWrite = controlLines[index] >> 6;
        int ALUsrc = (controlLines[index] >> 4) & 3;
        int MemToReg = (controlLines[index] >> 3) & 1;
        int MemRead = (controlLines[index] >> 2) & 1;
        int MemWrite = (controlLines[index] >> 1) & 1;
        int PC_select = (controlLines[index] & 1);
        int CSR_enable = 0;
        index++;
        signals.setControlLines(x, 0, 0, 0);
        EXPECT_EQ(regWrite, signals.get_RegWrite());
        EXPECT_EQ(ALUsrc, signals.get_ALUSrc());
        EXPECT_EQ(MemToReg, signals.get_MemtoReg());
        EXPECT_EQ(MemRead, signals.get_MemRead());
        EXPECT_EQ(MemWrite, signals.get_MemWrite());
        EXPECT_EQ(PC_select, signals.get_PC_Select());
        EXPECT_EQ(CSR_enable, signals.get_CSR_enable());
    }

    // setControlLines(int opcode, int csr_interrupt, int funct3, int supervisor_mode)
    signals.setControlLines(0b1110011, 0, 0b001, 1); // csrrw
    EXPECT_EQ(1, signals.get_CSR_enable());

    signals.setControlLines(0b1110011, 0, 0b010, 0); // csrrs
    EXPECT_EQ(0, signals.get_CSR_enable());

    signals.setControlLines(0b0110011, 1, 0b000, 0); // machine timer interrupt
    EXPECT_EQ(0x80000007, signals.get_mcause());
    EXPECT_EQ(1, signals.get_trap());

    signals.setControlLines(0b0110011, 2, 0b000, 0); // machine external interrupt
    EXPECT_EQ(0x8000000b, signals.get_mcause());
    EXPECT_EQ(1, signals.get_trap());

    signals.setControlLines(0b1110011, 0, 0b000, 0); // user mode ecall
    EXPECT_EQ(1, signals.get_trap());
    EXPECT_EQ(0x8, signals.get_mcause());
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

    

    uint32_t opcode = 0b0110111; // lui
    uint32_t instruction = 0xabcdf000 | opcode;
    uint32_t immediate = instruction & 0xfffff000;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    opcode = 0b0010111; // auipc
    instruction = 0xfdcba000 | opcode;
    immediate = instruction >> 12;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    opcode = 0b1100111; // jalr
    instruction = 0xfff00000 | opcode;
    immediate = instruction >> 20;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    opcode = 0b0000011; // load
    instruction = 0xaaa00000 | opcode;
    immediate = instruction >> 20;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    opcode = 0b0100011; // store
    uint32_t leftImmediate = 0b1111111 << 25;
    uint32_t rightImmediate = 0b11111 << 7;
    immediate = (leftImmediate >> 20) + (rightImmediate >> 7);
    instruction = leftImmediate | rightImmediate | opcode;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    opcode = 0b0010011; // i-type
    immediate = 0xcba;
    instruction = (immediate << 20) | opcode;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    // B-type
    leftImmediate = 0b1001011;
    rightImmediate = 0b11000;
    opcode = 0b1100011;
    instruction = (leftImmediate << 25) + (rightImmediate << 7) + opcode;
    immediate = ((leftImmediate << 5) + (rightImmediate));
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    // jal
    opcode = 0b1101111;
    immediate = 0xbbbff;
    instruction = (immediate << 12) | opcode;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));

    opcode = 0b1110011; // ecall
    immediate = 0x001;
    instruction = (immediate << 20) | opcode;
    EXPECT_EQ(immediate, imm.getImmediate(instruction));
}

TEST(CSR, csr_test) {
    CSR csr;
    // 0x000: mie               - Machine Interrupt Enable
    // 0x001: mpi               - Machine Pending Interrupt Enable
    // 0x002: mepc              - Machine Exception Program Counter
    // 0x003: mtvec             - Machine Trap Vector
    // 0x004: mcause            - Machine Cause (of trap)
    // 0x005: mode              - Mode of CPU (user/supervisor/machine)
    // 0x006: modep             - Previous Mode of CPU before trap
    // 0x007: mtimecmp_low      - Machine Time Compare (lower 32 bits)
    // 0x008: mtimecmp_high     - Machine Time Compare (upper 32 bits)
    
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

// TEST(Memory, read_write_test) {
//     Memory dram;
//     uint32_t data;
//     uint32_t address;
//     // int MemReadData, int MemWriteData, int size, int IO_WR_enable
//     dram.set_control_signals(1, 1, 4, 0);
//     address = 0x0000ffff;
//     data = 0x00005555;
//     dram.write_data(address, data);
//     EXPECT_EQ(data, dram.read_data(address));
//     data = 0x0000abcd;
//     dram.write_data(address, data);
//     EXPECT_EQ(data, dram.read_data(address));
//     address = 0xffffffff;
//     EXPECT_EQ(0, dram.read_data(address));
    
//     address = 0x000000ff;
//     data = 0x12345678;
//     dram.write_io(address, data);
//     EXPECT_EQ(0, dram.read_data(address));

//     dram.set_control_signals(1, 1, 4, 1);
//     dram.write_io(address, data);
//     EXPECT_EQ(data, dram.read_data(address));
// }

// TEST(NextPC, next_PC) {
//     NextPC<uint32_t> nextPC;
//     nextPC.setCurrentPC(0x0);
//     uint32_t offset = 2;
//     // uint32_t opcode = 0b0110111;
//     uint32_t funct3 = 0b000;
//     uint32_t A = 0x3;
//     uint32_t B = 0x3;
//     uint32_t interrupt_taken = 0;
//     uint32_t mtvec = 0xfff;

//     // lui / auipc / load / store / i-type / r-type
//     uint32_t opcode[6] = {0b0110111, 0b0010011, 0b0110011, 0b0000011, 0b0100011, 0b0010111};
//     uint32_t expected_addr = 0;
//     for (int x : opcode) {
//         expected_addr += 4;
//         nextPC.calculateNextPC(offset, x, funct3, A, B, mtvec, interrupt_taken);
//         EXPECT_EQ(expected_addr, nextPC.getNextPC());
//     }

//     // b-type - beq, bne, blt, bge, bltu, bgeu
//     uint32_t funct3_b[6] = {0b000, 0b001, 0b100, 0b101, 0b110, 0b111};
//     nextPC.setCurrentPC(0);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[0], A, B, mtvec, interrupt_taken); // beq
//     EXPECT_EQ(0x8, nextPC.getNextPC());
//     A = 0x3;
//     B = 0x2;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[1], A, B, mtvec, interrupt_taken); // bne
//     EXPECT_EQ(0x10, nextPC.getNextPC());
//     A = 0x80000000;
//     B = 0x0;
//     offset = 4;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[2], A, B, mtvec, interrupt_taken); // blt
//     EXPECT_EQ(32, nextPC.getNextPC());
//     offset = -4;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[2], A, B, mtvec, interrupt_taken); // blt
//     EXPECT_EQ(0x10, nextPC.getNextPC());
//     offset = 4;
//     A = 0x3;
//     B = 0x3;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[2], A, B, mtvec, interrupt_taken); // blt
//     EXPECT_EQ(0x14, nextPC.getNextPC());
//     offset = 2;
//     A = 0xff;
//     B = 0xff;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[3], A, B, mtvec, interrupt_taken); // bge
//     EXPECT_EQ(0x1c, nextPC.getNextPC());
//     A = 0xf;
//     B = 0x80000000;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[3], A, B, mtvec, interrupt_taken); // bge
//     EXPECT_EQ(0x20, nextPC.getNextPC());
//     A = 0xf;
//     B = 0xff;
//     offset = 10;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[0], A, B, mtvec, interrupt_taken); // beq
//     EXPECT_EQ(0x24, nextPC.getNextPC());
//     A = 0xf;
//     B = 0xf;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[1], A, B, mtvec, interrupt_taken); // bne
//     EXPECT_EQ(0x28, nextPC.getNextPC());
    
//     // fix bltu / bgeu
//     A = 0x80000000;
//     B = 0xf;
//     offset = 10;
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[4], A, B, mtvec, interrupt_taken); // bltu
//     EXPECT_EQ(0x2c, nextPC.getNextPC());

//     A = 0xf;
//     B = 0x80000000;
//     expected_addr = 0x2c + (10 << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[4], A, B, mtvec, interrupt_taken); // bltu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xf;
//     B = 0xff;
//     offset = 2;
//     expected_addr = expected_addr + (offset << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[4], A, B, mtvec, interrupt_taken); // bltu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xff;
//     B = 0xf;
//     expected_addr = expected_addr + (1 << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[4], A, B, mtvec, interrupt_taken); // bltu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xffffffff;
//     B = 0xfffffffe;
//     offset = 4;
//     expected_addr += (1 << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[4], A, B, mtvec, interrupt_taken); // bltu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xfffffffe;
//     B = 0xffffffff;
//     expected_addr += (offset << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[4], A, B, mtvec, interrupt_taken); // bltu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xf;
//     B = 0xf;
//     expected_addr += (1 << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[4], A, B, mtvec, interrupt_taken); // bltu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     expected_addr += (offset << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[5], A, B, mtvec, interrupt_taken); // bgeu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xff;
//     B = 0xf;
//     expected_addr += (offset << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[5], A, B, mtvec, interrupt_taken); // bgeu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xffffffff;
//     B = 0x0;
//     expected_addr += (offset << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[5], A, B, mtvec, interrupt_taken); // bgeu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     A = 0xf;
//     B = 0xff;
//     expected_addr += (1 << 2);
//     nextPC.calculateNextPC(offset, 0b1100011, funct3_b[5], A, B, mtvec, interrupt_taken); // bgeu
//     EXPECT_EQ(expected_addr, nextPC.getNextPC());

//     // jal
//     nextPC.setCurrentPC(0x4);
//     offset = 10;
//     nextPC.calculateNextPC(offset, 0b1101111, 0, A, B, mtvec, interrupt_taken);
//     EXPECT_EQ(44, nextPC.getNextPC());
//     offset = -2;
//     nextPC.calculateNextPC(offset, 0b1101111, 0, A, B, mtvec, interrupt_taken);
//     EXPECT_EQ(36, nextPC.getNextPC());
//     offset = 4;
//     nextPC.calculateNextPC(offset, 0b1101111, 0, A, B, mtvec, interrupt_taken);
//     EXPECT_EQ(52, nextPC.getNextPC());

//     // jalr
//     A = 8; // rs1 pointing to byte 8 (beginning of 2nd instruction)
//     offset = 4; // jump to byte 8 + (4 << 2) ---- 6th instruction
//     nextPC.calculateNextPC(offset, 0b1100111, 0, A, B, mtvec, interrupt_taken);
//     EXPECT_EQ(24, nextPC.getNextPC());

//     // ecall
//     interrupt_taken = 1;
//     nextPC.calculateNextPC(offset, 0b1110011, 0, A, B, mtvec, interrupt_taken);
//     EXPECT_EQ(mtvec, nextPC.getNextPC());

//     // csr instruction
//     interrupt_taken = 0;
//     nextPC.calculateNextPC(offset, 0b1110011, 0b001, A, B, mtvec, interrupt_taken);
//     EXPECT_EQ(mtvec + 4, nextPC.getNextPC());
// }

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
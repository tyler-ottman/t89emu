// #include <iostream>
// #include <vector>
// #include <deque>

// #include <gtest/gtest.h>
// #include "Components.h"

// // ALU Module Operations Test
// TEST(ALU, ALU_test)  {
//     ALU alu;
//     uint32_t res;
    
//     /*****************************************
//      ************ ALU OPERATION **************
//      ************  1ST OPERAND  **************
//      ************  2ND OPERAND  **************
//      ****************************************/
//     std::vector<std::vector<uint32_t>> arguments = {
//         // ADD
//         {0x0000000f, 0x0000f3e9, 0x000000ff, 0x000000ff},
//         {0x000000ff, 0xffffffff, 0x00000001, 0x00000000},
//         // SUB
//         {0x0000001f, 0xffffffff, 0x00000000},
//         {0x0000000f, 0x0000000f, 0x0000000f},
//         // OR
//         {0x00000006, 0x8000000f},
//         {0x00000009, 0xf000000f},
//         // AND
//         {0x00008000, 0x00000003},
//         {0x0000f001, 0x00000006},
//         // XOR
//         {0x00008000, 0x0000ffff},
//         {0x00007fff, 0x0000ffff},
//         // SRL
//         {0x00000008, 0x80000000, 0xffffffff},
//         {0x00000001, 0x0000001f, 0x00000028},
//         // SRA
//         {0x80000000, 0x80000000, 0x00000008},
//         {0x00000001, 0x00000003, 0x00000002},
//         // SLL
//         {0x00000001, 0x00000001, 0x00000001},
//         {0x00000001, 0x0000001f, 0x00000020},
//         // SLT
//         {0x00000001, 0xffffffff, 0x80000001, 0x00000004, 0x80000001, 0xffffffff, 0x0000000a},
//         {0x00000002, 0x00000005, 0x00000001, 0x00000004, 0xffffffff, 0x80000001, 0x00000005},
//         // SLTU
//         {0x00000003, 0xfffffffe, 0xffffffff, 0x0000000f, 0x00000fff, 0xfffffff0, 0x80000000},
//         {0x00000005, 0xffffffff, 0x80000000, 0xffffffff, 0x80000000, 0x00000fac, 0x00000001}
//     };

//     // Results that can't be calculated with operators must be specified manually (SRA, SLL, SLT, SLTU)
//     std::deque<uint32_t> r_sra  = {0xc0000000, 0xf0000000, 0x00000002};
//     std::deque<uint32_t> r_slt  = {0x00000001, 0x00000001, 0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000000};
//     std::deque<uint32_t> r_sltu = {0x00000001, 0x00000001, 0x00000000, 0x00000001, 0x00000001, 0x00000000, 0x00000000};

//     // Error check
//     for (size_t i = 0; i < arguments.size(); i += 2) {
//         size_t arg_size = arguments.at(i).size();
//         int ALU_op = i / 2;
//         // Special operations result size check
//         if (((ALU_op == (size_t)SRA)  && (arg_size != r_sra.size()) ) ||
//             ((ALU_op == (size_t)SLT)  && (arg_size != r_slt.size()) ) ||
//             ((ALU_op == (size_t)SLTU) && (arg_size != r_sltu.size()))) {
//                 std::cerr << "Special results: Invalid Test Dimensions" << std::endl;
//                 exit(EXIT_FAILURE);
//         }
//         // Verify operand dimensions
//         if (arguments.at(i).size() != arguments.at(i + 1).size()) {
//             std::cerr << "Operands: Invalid Test Dimensions" << std::endl;
//             exit(EXIT_FAILURE);
//         }
//     }

//     // Test ALU operations
//     for (size_t i = 0; i < arguments.size(); i += 2) {
//         int ALU_op = i / 2;
//         for (size_t j = 0; j < arguments.at(i).size(); j++) {
//             switch (ALU_op)
//             {
//             case ADD:
//                 res = arguments.at(i).at(j) + arguments.at(i + 1).at(j);
//                 break;
//             case SUB:
//                 res = arguments.at(i).at(j) - arguments.at(i + 1).at(j);
//                 break;
//             case OR:
//                 res = arguments.at(i).at(j) | arguments.at(i + 1).at(j);
//                 break;
//             case AND:
//                 res = arguments.at(i).at(j) & arguments.at(i + 1).at(j);
//                 break;
//             case XOR:
//                 res = arguments.at(i).at(j) ^ arguments.at(i + 1).at(j);
//                 break;
//             case SRL:
//                 res = arguments.at(i).at(j) >> arguments.at(i + 1).at(j);
//                 break;
//             case SRA:
//                 res = r_sra.at(0); 
//                 r_sra.pop_front();
//                 break;
//             case SLL:
//                 res = arguments.at(i).at(j) << arguments.at(i + 1).at(j);
//                 break;
//             case SLT:
//                 res = r_slt.at(0); 
//                 r_slt.pop_front();
//                 break;
//             case SLTU:
//                 res = r_sltu.at(0);
//                 r_sltu.pop_front();
//                 break;
//             default:
//                 std::cout << "Unexpected error" << std::endl;
//                 exit(EXIT_FAILURE);
//             }
//             EXPECT_EQ(res, alu.execute(arguments.at(i).at(j), arguments.at(i + 1).at(j), ALU_op));   
//         }
//     }
// }

// // Test ALU Control Unit
// TEST(ALUControlUnit, Misc_Instr) {
//     ALUControlUnit misc;
//     int ALUop[6] = {0, 1, 2, 3, 5, 6}; // lui, add, jal, jalr, loads, stores
//     int funct7 = 0;
//     int funct3 = 0;
//     for (int x : ALUop) {
//         EXPECT_EQ(0, misc.getALUOperation(x, funct7, funct3));
//     }
// }

// TEST(ALUControlUnit, ALUControlUnit) {
//     ALUControlUnit alucu;

//     // addi, slti, sltiu, xori, ori, andi, slli
//     std::vector<int> funct3 = {0b000, 0b010, 0b011, 0b100, 0b110, 0b111, 0b001};
//     std::vector<int> funct7 = {0};
//     int index = 0;
//     std::vector<int> expected = {0, 8, 9, 4, 2, 3, 7}; // Expected ALU operation code
//     for (int x : funct3) {
//         int ALU_operation = alucu.getALUOperation(ITYPE, funct7.at(0), x);
//         EXPECT_EQ(expected.at(index++), ALU_operation);
//     }

//     // srli/srai
//     funct7 = {0b0000000, 0b0100000};
//     EXPECT_EQ(5, alucu.getALUOperation(ITYPE, funct7.at(0), 0b101));
//     EXPECT_EQ(6, alucu.getALUOperation(ITYPE, funct7.at(1), 0b101));

//     // Branch
//     EXPECT_EQ(1, alucu.getALUOperation(BTYPE, 0, 0));

//     // add / sub R-type
//     funct7 = {0b0000000, 0b0100000};
//     expected = {0, 1};
//     index = 0;
//     for (int x : funct7) {
//         EXPECT_EQ(expected[index++], alucu.getALUOperation(RTYPE, x, 0b000));
//     }

//     // SRL / SRA R-type
//     funct7 = {0b0000000, 0b0100000};
//     expected = {5, 6};
//     index = 0;
//     for (int x : funct7) {
//         EXPECT_EQ(expected[index++], alucu.getALUOperation(RTYPE, x, 0b101));
//     }

//     // sll, slt, sltu, xor, or, and
//     funct3 = {0b001, 0b010, 0b011, 0b100, 0b110, 0b111};
//     expected = {7, 8, 9, 4, 2, 3};
//     index = 0;
//     for (int x : funct3) {
//         EXPECT_EQ(expected[index++], alucu.getALUOperation(RTYPE, 0, x));
//     }
// }

// // Register File
// TEST(RegisterFile, read_write) {
//     RegisterFile rf;
//     int reg = 10;
//     uint32_t data = 0xff;
//     rf.write(data, reg);
//     EXPECT_EQ(data, rf.read(reg));
    
//     data = 20;
//     rf.write(data, 0);
//     EXPECT_EQ(0, rf.read(0));
// }

// TEST(ProgramCounter, setPC) {
//     ProgramCounter pc;
//     pc.setPC(0xfff);
//     EXPECT_EQ(0xfff, pc.getPC());
// }

// TEST(ImmediateGenerator, immediates) {
//     ImmediateGenerator imm;
//     uint32_t instruction;

//     // Upper Immediate Instructions (jal here)
//     std::vector<uint32_t> opcode =    {LUI,     AUIPC};
//     std::vector<uint32_t> immediate = {0xabcdf, 0xfdcba};
//     for (size_t i = 0; i < opcode.size(); i++) {
//         if (opcode[i] == AUIPC || opcode[i] == LUI) {
//             immediate[i] = immediate[i] << 12;
//             instruction = immediate[i] | opcode[i];
//             EXPECT_EQ(immediate[i], imm.getImmediate(instruction));
//         }
//     }
//     // Stores (B-Type here)
//     opcode =                     {STORE,     STORE};
//     std::vector<int> left_imm  = {0b0111111, 0b1111111};
//     std::vector<int> right_imm = {0b11111,   0b11111};
//     for (size_t i = 0; i < opcode.size(); i++) {
//         instruction = (left_imm[i] << 25) + (right_imm[i] << 7) + opcode[i];
//         immediate[i] = ((left_imm[i] << 5) + (right_imm[i]));
//         if ((left_imm[i] >> 6)) {
//             immediate[i] |= 0xfffff000;
//         }
//         EXPECT_EQ(immediate[i], imm.getImmediate(instruction));
//     }

//     // Jalr, Load, I-type
//     opcode    = {JALR,  LOAD,  ITYPE};
//     immediate = {0xfff, 0xaaa, 0xcba};
//     for (size_t i = 0; i < opcode.size(); i++) {
//         instruction = (immediate[i] << 20) | opcode[i];
//         if (instruction >> 31) {
//             immediate[i] |= 0xfffff000;
//         }
//         EXPECT_EQ(immediate[i], imm.getImmediate(instruction));
//     }

//     // ecall
//     instruction = (0x001 << 20) | ECALL;
//     EXPECT_EQ(0x001, imm.getImmediate(instruction));
// }

// // TEST(CSR, csr_test) {
// //     CSR csr;
// //     // csr_we, trap_taken, mcause, pc
// //     csr.set_control_lines(1, 0, 0, 0); // csrrw
// //     csr.update_csr(0x000, 1);
// //     EXPECT_EQ(1, csr.get_csr(0x000));

// //     csr.set_control_lines(0, 0, 0, 0); // csrrs
// //     csr.update_csr(0x000, 0);
// //     EXPECT_EQ(1, csr.get_csr(0x000));

// //     // Trap Taken (time interrupt when pc at 0x0000ffff)
// //     uint32_t pc = 0x0000ffff;
// //     uint32_t mcause = 0x80000007;
// //     csr.set_control_lines(0, 1, mcause, pc);
// //     EXPECT_EQ(mcause, csr.get_csr(0x004));
// //     EXPECT_EQ(pc, csr.get_csr(0x002));
// //     EXPECT_EQ(0, csr.get_csr(0x000));
// //     EXPECT_EQ(0, csr.get_csr(0x001));
// // }

// #define WRITE 0
// #define WRITE_IO 1
// #define READ 2

// TEST(Memory, read_write_test) {
//     Memory dram;
//     // int MemReadData, int MemWriteData, int size, int IO_WR_enable

//     // Word Read/Write
//     std::vector<uint32_t> address = {0x0000ffff, 0x0000ffff, 0x000000ff, 0x000000ff, 0x0000000f};
//     std::vector<uint32_t> data    = {0x00000020, 0x00000020, 0x0000000a, 0x0000000a, 0x00000000};
//     std::vector<uint32_t> mem_op  = {WRITE,      READ,       WRITE,      READ,       READ};
//     std::vector<uint32_t> size    = {4,          4,          4,          4,          4};
//     for (size_t i = 0; i < mem_op.size(); i++) {
//         switch(mem_op[i]) {
//             case WRITE: dram.write(address.at(i), data.at(i), size.at(i)); break;
//             case READ: EXPECT_EQ(data.at(i), dram.read(address.at(i), size.at(i))); break;
//         }
//     }

//     // Halfword Read/Write
//     address = {0x00000002, 0x00000000, 0x00000008, 0x00000008};
//     data    = {0x0000ffff, 0xffff0000, 0x0000abcd, 0x0000abcd};
//     mem_op  = {WRITE,      READ,       WRITE,      READ};
//     size    = {2,          4,          2,          4};
//     for (size_t i = 0; i < mem_op.size(); i++) {
//         switch(mem_op[i]) {
//             case WRITE: dram.write(address.at(i), data.at(i), size.at(i)); break;
//             case READ: EXPECT_EQ(data.at(i), dram.read(address.at(i), size.at(i))); break;
//         }
//     }

//     // Byte Read/Write
//     address = {0x000000f3, 0x000000f0, 0x000000f2, 0x000000f1, 0x000000f0};
//     data    = {0x0000000a, 0x0a000000, 0x00000a00, 0x0000000d, 0x0a000d00};
//     mem_op  = {WRITE,      READ,       READ,       WRITE,      READ};
//     size    = {1,          4,          2,          1,          4};
//     for (size_t i = 0; i < mem_op.size(); i++) {
//         switch(mem_op[i]) {
//             case WRITE: dram.write(address.at(i), data.at(i), size.at(i)); break;
//             case READ: EXPECT_EQ(data.at(i), dram.read(address.at(i), size.at(i))); break;
//         }
//     }
// }

// TEST(MemControlUnit, size_test) {
//     MemControlUnit mcu;
//     std::vector<uint32_t> funct3 = {0b000, 0b001, 0b010, 0b100};
//     std::vector<uint32_t> expect = {1,     2,     4,     4};

//     for (size_t i = 0; i < funct3.size(); i++) {
//         EXPECT_EQ(expect.at(i), mcu.get_mem_size(funct3.at(i)));
//     }
// }

// int main(int argc, char* argv[]) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
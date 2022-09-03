#include "ALUControlUnit.h"

ALUControlUnit::ALUControlUnit() {
    //                    addi,   slli,   slti,   sltiu,  xori,   srl/ri, ori,    andi
    this->i_operations = {0b0000, 0b0111, 0b1000, 0b1001, 0b0100, 0xeeee, 0b0010, 0b0011};
    //                    +/-,    sll,    slt,    sltu,   xor,    srl/a,  or,     and
    this->r_operations = {0xeeee, 0b0111, 0b1000, 0b1001, 0b0100, 0xeeee, 0b0010, 0b0011};
}

int ALUControlUnit::getALUoperation(int opcode, int funct7, int funct3) {
    switch (opcode) {
    case ITYPE: // I-Type
        switch (funct3) {
        case 5: //                         srli,    srai
            return (funct7 == 0b0000000) ? 0b0101 : 0b0110;
        default: // all other I-Type instructions
            return this->i_operations[funct3];
        }
    case RTYPE: // R-Type
        switch (funct3) {
        case 0: //                         add,     sub
            return (funct7 == 0b0000000) ? 0b0000 : 0b0001;
        case 5: //                         srl,     sra
            return (funct7 == 0b0000000) ? 0b0101 : 0b0110;
        default: // all other R-Type instructions
            return this->r_operations[funct3];
        }
    case BTYPE: // B-Type
        return 0b0001;
    default: // lui, auipc, jal, jalr, loads, stores
        return 0b0000;
    }
}
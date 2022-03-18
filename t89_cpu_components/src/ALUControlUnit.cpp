#include <iostream>
#include "Components.h"

int i_type;
int r_type;

int ALUControlUnit::get_i_type(int funct3, int funct7)
{
    int i_type;
    switch (funct3)
    {
    case 0b000: // addi
        i_type = 0;
        break;
    case 0b010: // slti
        i_type = 0b1000;
        break;
    case 0b011: // sltiu
        i_type = 0b1001;
        break;
    case 0b100: // xori
        i_type = 0b0100;
        break;
    case 0b110: // ori
        i_type = 0b0010;
        break;
    case 0b111: // andi
        i_type = 0b0011;
        break;
    case 0b001: // slli
        i_type = 0b0111;
        break;
    case 0b101: // srli/srai
        switch (funct7)
        {
        case 0b0000000: // srli
            i_type = 0b0101;
            break;
        case 0b0100000: // srai
            i_type = 0b0110;
            break;
        default:
            std::cerr << "Incorrect funct7 Field" << std::endl;
            exit(1);
        }
        break;
    default:
        std::cerr << "Incorrect funct3/funct7 Field" << std::endl;
        exit(1);
    }
    return i_type;
}

int ALUControlUnit::get_r_type(int funct3, int funct7)
{
    int r_type;
    switch (funct3)
    {
    case 0b000: // add/sub
        switch (funct7)
        {
        case 0b0000000: // add
            r_type = 0b0000;
            break;
        case 0b0100000: // sub
            r_type = 0b0001;
            break;
        }
        break;
    case 0b001: // sll
        r_type = 0b0111;
        break;
    case 0b010: // slt
        r_type = 0b1000;
        break;
    case 0b011: // sltu
        r_type = 0b1001;
        break;
    case 0b100: // xor
        r_type = 0b0100;
        break;
    case 0b101: // srl/sra
        switch (funct7)
        {
        case 0b0000000: // srl
            r_type = 0b0101;
            break;
        case 0b0100000: // sra
            r_type = 0b0110;
            break;
        default:
            std::cerr << "Incorrect funct7 Field" << std::endl;
            exit(1);
        }
        break;
    case 0b110: // or
        r_type = 0b0010;
        break;
    case 0b111: // and
        r_type = 0b0011;
        break;
    default:
        std::cerr << "Incorrect funct3/funct7 Field" << std::endl;
        exit(1);
    }
    return r_type;
}

ALUControlUnit::ALUControlUnit() {}

int ALUControlUnit::getALUoperation(int ALUop, int funct7, int funct3)
{
    int ALU_operation;
    switch (ALUop)
    {
    case 0: // lui (add)
        ALU_operation = 0;
        break;
    case 1: // auipc (add)
        ALU_operation = 0;
        break;
    case 2: // jal (add)
        ALU_operation = 0;
        break;
    case 3: // jalr (add)
        ALU_operation = 0;
        break;
    case 4: // B-Type (sub)
        ALU_operation = 1;
        break;
    case 5: // Loads (add)
        ALU_operation = 0;
        break;
    case 6: // Stores (add)
        ALU_operation = 0;
        break;
    case 7: // I-Type (varies)
        ALU_operation = get_i_type(funct3, funct7);
        break;
    case 8: // R-Type (varies)
        ALU_operation = get_r_type(funct3, funct7);
        break;
    }

    return ALU_operation;
}

class ALUControlUnit;
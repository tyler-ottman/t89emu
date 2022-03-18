#include <iostream>
#include <stdlib.h>
#include "Components.h"

int opcode;
int RegWrite;
int ALUSrc;
int MemtoReg;
int MemRead;
int MemWrite;
int PC_Select;
int CSR_enable;
int intr_taken;
uint32_t mcause = 0;


int ControlUnit::getInstructionType()
{
    int instr_type;
    int index = 0;
    // lui, auipc, jal, jalr, B-type, loads, stores, i-type, r-type, ecall/csr
    int opcodes[10] = {0b0110111, 0b0010111, 0b1101111, 0b1100111, 0b1100011, 0b0000011, 0b0100011, 0b0010011, 0b0110011, 0b1110011};
    while(index != 10) {
        if (this->opcode == opcodes[index]) {
            instr_type = index;
            break;
        }
        index++;
    }
    if (index == 10) {
        // invalid opcode
        std::cerr << "Invalid Opcode Argument" << std::endl;
        exit(EXIT_FAILURE);
    }
    return instr_type;
}

void ControlUnit::setControlLines(int opcode, int csr_interrupt, int funct3, int supervisor_mode)
{
    this->opcode = opcode;
    // lui, auipc, jal, jalr, b-type, load, store, i-type, r-type, csr
    int RegWrite[10]   = {0b01, 0b01, 0b01, 0b01, 0b00, 0b01, 0b00, 0b01, 0b01, 0b00};
    int ALUSrc[10]     = {0b10, 0b10, 0b00, 0b00, 0b01, 0b10, 0b10, 0b10, 0b01, 0b00};
    int MemtoReg[10]   = {0b00, 0b00, 0b00, 0b00, 0b00, 0b01, 0b00, 0b00, 0b00, 0b10};
    int MemRead[10]    = {0b00, 0b00, 0b00, 0b00, 0b00, 0b01, 0b00, 0b00, 0b00, 0b00};
    int MemWrite[10]   = {0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b01, 0b00, 0b00, 0b00};
    int PC_Select[10]  = {0b00, 0b01, 0b01, 0b01, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00};

    // 0: lui
    // 1: auipc
    // 2: jal
    // 3: jalr
    // 4: b-type
    // 5: load
    // 6: store
    // 7: i-type
    // 8: r-type
    // 9: ecall/csr
    int instr_type = getInstructionType();

    this->RegWrite = RegWrite[instr_type];
    this->ALUSrc = ALUSrc[instr_type];
    this->MemtoReg = MemtoReg[instr_type];
    this->MemRead = MemRead[instr_type];
    this->MemWrite = MemWrite[instr_type];
    this->PC_Select = PC_Select[instr_type];
    this->CSR_enable = 0; // csrrs / ecall / all other instructions
    if(funct3 == 0b001 && instr_type == 9 && supervisor_mode)
        this->CSR_enable = 1; // csrrw
    this->intr_taken = 0;

    // Check for trap
    if ((instr_type == 9 && funct3 == 0b000) || csr_interrupt) {
        this->intr_taken = 1;
        this->RegWrite = 0;
        this->ALUSrc = 0;
        this->MemtoReg = 0;
        this->MemRead = 0;
        this->MemWrite = 0;
        this->PC_Select = 0;
        this->CSR_enable = 0;

        // update trap cause
        if (instr_type == 9) // ecall from user mode
            this->mcause = 0x00000008;
        else if (csr_interrupt == 1) // machine timer interrupt
            this->mcause = 0x80000007;
        else if (csr_interrupt == 2) // machine external interrupt
            this->mcause = 0x8000000b;
        return;
    }
}

ControlUnit::ControlUnit(){}

int ControlUnit::get_RegWrite()
{
    return this->RegWrite;
}

int ControlUnit::get_ALUSrc()
{
    return this->ALUSrc;
}

int ControlUnit::get_MemtoReg()
{
    return this->MemtoReg;
}

int ControlUnit::get_MemRead()
{
    return this->MemRead;
}

int ControlUnit::get_MemWrite()
{
    return this->MemWrite;
}

int ControlUnit::get_PC_Select()
{
    return this->PC_Select;
}

int ControlUnit::get_CSR_enable()
{
    return this->CSR_enable;
}

int ControlUnit::get_intr_taken()
{
    return this->intr_taken;
}

int ControlUnit::get_mcause()
{
    return this->mcause;
}

class ControlUnit;
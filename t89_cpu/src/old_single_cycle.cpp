#include "../../t89_cpu_components/include/Components.h"
#include <iostream>

int main(void) {    
    int num_instruction = 1;
    int cur_instrutions_index = 0;
    int cur_instruction;

    uint32_t instructions[1] = {0x00f58593};
    // addi a1, a1, 0xf

    ProgramCounter pc;
    RegisterFile<uint32_t> rf;
    ImmediateGenerator<uint32_t> imm;
    ControlUnit mcu;
    ALUControlUnit acu;
    NextPC<uint32_t> npc;
    ALU<uint32_t> alu;

    do {
        // Fetch Instruction
        cur_instruction = instructions[cur_instrutions_index];
        std::cout << "Current instruction: " << cur_instruction << std::endl;
        
        
    } while(++cur_instrutions_index < num_instruction);

    exit(EXIT_SUCCESS);
}
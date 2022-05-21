#include "Components.h"
#include <map> 
#include <iostream>
#include <fstream>
#include <vector>

#define ROM_START 0x00000000
#define RAM_START 0x80000000

class CPU {
private:
    uint32_t IO_BUS;
	uint32_t IO_we;
	uint32_t IO_addr;
	int interrupt_assert;
	int debug;
    int counter;

	ALU alu;
	ALUControlUnit aluc;
	ControlUnit ctrl;
	CSR csr;
	ImmediateGenerator<uint32_t> immgen;
	Memory dram;
	NextPC<uint32_t> nextpc;
	ProgramCounter pc;
	RegisterFile<uint32_t> rf;
public:
    CPU(std::multimap<uint32_t, uint32_t> memory, int debug = 0)
    {
        this->counter = 0;
        this->interrupt_assert = 0;
        this->IO_BUS = 0;	 // 32 bit IO bus
        this->IO_we = 0;	 // IO write enable
        this->IO_addr = 0;	 // Address of device
        this->debug = debug; // Debug mode

        // Load Memory
        dram.set_control_signals(1, 1, 4, 1);
        for (std::multimap<uint32_t, uint32_t>::iterator it = memory.begin(); it != memory.end(); it++)
        {
            dram.write_data(it->first, it->second);
        }
    }

    void debug_pre_execute(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t rs1, uint32_t rs2, uint32_t rd, uint32_t immediate, uint32_t csr_addr, uint32_t cur_instruction)
    {
        std::cout << "Current Instruction: " << cur_instruction << std::endl;
        switch (opcode)
        {
        case 0b0110111: // lui
            std::cout << "immediate: " << immediate;
            std::cout << " rd: " << rd;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b0010011: // I-type
            std::cout << "immediate: " << immediate;
            std::cout << " rs1: " << rs1;
            std::cout << " funct3: " << funct3;
            std::cout << " rd: " << rd;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b0110011: // R-type
            std::cout << "rs2: " << rs2;
            std::cout << " rs1: " << rs1;
            std::cout << " funct3: " << funct3;
            std::cout << " rd: " << rd;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b0000011: // Load
            std::cout << "immediate: " << immediate;
            std::cout << " rs1: " << rs1;
            std::cout << " funct3: " << funct3;
            std::cout << " rd: " << rd;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b0100011: // Store
            std::cout << "immediate: " << immediate;
            std::cout << " rs2: " << rs2;
            std::cout << " rs1: " << rs1;
            std::cout << " funct3: " << funct3;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b1100011: // B-Type
            std::cout << "immediate: " << immediate;
            std::cout << " rs2: " << rs2;
            std::cout << " rs1: " << rs1;
            std::cout << " funct3: " << funct3;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b1101111: // jal
            std::cout << "immediate: " << immediate;
            std::cout << " rd: " << rd;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b0010111: // auipc
            std::cout << "immediate: " << immediate;
            std::cout << " rd: " << rd;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b1100111: // jalr
            std::cout << "immediate: " << immediate;
            std::cout << " rd: " << rd;
            std::cout << " rs: " << rs1;
            std::cout << " opcode: " << opcode << std::endl;
            break;
        case 0b1110011: // ecall/csr
            if (funct3 == 0)
            {
                std::cout << "ecall" << std::endl;
            }
            break;
        }
    }

    void debug_post_execute(uint32_t opcode, uint32_t rd, uint32_t immediate)
    {
        switch (opcode)
        {
        case 0b0110111: // lui
            std::cout << "Wrote " << rf.read_rd() << " to register " << rd << std::endl
                      << std::endl;
            break;
        case 0b0010011: // I-Type
            std::cout << "Wrote " << rf.read_rd() << " to register " << rd << std::endl
                      << std::endl;
            break;
        case 0b0110011: // R-type
            std::cout << "Wrote " << rf.read_rd() << " to register " << rd << std::endl
                      << std::endl;
            break;
        case 0b0000011: // Load
            std::cout << "Wrote " << rf.read_rd() << " to register " << rd << std::endl
                      << std::endl;
            break;
        case 0b0100011: // Store
            std::cout << "Wrote " << rf.read_rs2() << " to address " << rf.read_rs1() + immediate << std::endl
                      << std::endl;
            break;
        case 0b1100011: // B-Type
            std::cout << "Next PC: " << pc.getPC() << std::endl
                      << std::endl;
            break;
        case 0b1101111: // jal
            std::cout << "Next PC: " << pc.getPC() << ". Wrote " << rf.read_rd() << " to register " << rd << std::endl
                      << std::endl;
            break;
        case 0b0010111: // auipc
            std::cout << "Wrote " << pc.getPC() - 4 + immediate << " to register " << rd << std::endl
                      << std::endl;
            break;
        case 0b1100111: // jalr
            std::cout << "Next PC: " << pc.getPC() << ". Wrote " << rf.read_rd() << " to register " << rd << std::endl
                      << std::endl;
            break;
        case 0b1110011: // ecall/csr
            std::cout << "mcause: " << csr.get_csr(4) << std::endl;
            std::cout << "mepc: " << csr.get_csr(2) << std::endl;
            std::cout << "Jump to handler at: " << pc.getPC() << std::endl
                      << std::endl;
            break;
        }
    }

    void executeInstruction()
    {
        this->counter++;
        if (this->counter > 10000000) {
            std::cout << "done" << std::endl;
            exit(1);
        }
        /********************************************
         * ************Microcontroller***************
         * *****************************************/
        int external_interrupt = 0;
        // query_external_interrupt(); // Check for external interrupt assertion
        if ((this->IO_we == 1) && (csr.get_csr(0x000) == 1))
        {
            // External Interrupt and MIE enabled
            dram.set_control_signals(0, 0, 4, this->IO_we);
            dram.write_io(this->IO_addr, this->IO_BUS); // write IO data to IO port
            external_interrupt = 1;
            this->IO_we = 0;
            this->interrupt_assert = 0;
        }

        /********************************************
         *******************FETCH********************
        *******************************************/
        uint32_t pc_addr = pc.getPC();						// Current PC
        uint32_t cur_instruction = dram.read_data(pc_addr); // Current Instruction

        /********************************************
         *******************DECODE*******************
        *******************************************/
        uint32_t opcode = cur_instruction & 0b1111111;			   // opcode field
        uint32_t funct3 = (cur_instruction >> 12) & 0b111;		   // funct3 field
        uint32_t funct7 = (cur_instruction >> 25) & 0b1111111;	   // funct7 field
        uint32_t mode = csr.get_csr(0x005);						   // mode field
        uint32_t rs1 = (cur_instruction >> 15) & 0b11111;		   // Register Source 1
        uint32_t rs2 = (cur_instruction >> 20) & 0b11111;		   // Register Source 2
        uint32_t rd = (cur_instruction >> 7) & 0b11111;			   // Register Desination
        uint32_t immediate = immgen.getImmediate(cur_instruction); // Instruction Immediate
        uint32_t csr_addr = (cur_instruction >> 20) & 0xfff;	   // CSR Address

        // debug_pre_execute(opcode, funct3, funct7, rs1, rs2, rd, immediate, csr_addr, cur_instruction);

        // Control Unit Signals
        std::vector<uint32_t> signals_bus = ctrl.getControlLines(opcode, external_interrupt, funct3, mode);
        uint32_t main_bus = signals_bus[0];
        uint32_t RegWrite  = (main_bus >> 10) & 0b11;
        uint32_t ALUSrc    = (main_bus >>  8) & 0b11;
        uint32_t MemToReg  = (main_bus >>  6) & 0b11;
        uint32_t MemRead   = (main_bus >>  4) & 0b11;
        uint32_t MemWrite  = (main_bus >>  2) & 0b11;
        uint32_t PC_select = (main_bus      ) & 0b11;
        uint32_t trap_taken = signals_bus[1];
        uint32_t mcause = signals_bus[2];
        uint32_t CSR_en = signals_bus[3];

        // Control Lines - CSR, Register File, ALU
        csr.set_control_lines(CSR_en, trap_taken, mcause, pc_addr);		   // CSR Control
        rf.set_control_lines(rs1, rs2, RegWrite);						   // Register File Control
        dram.set_control_signals(MemRead, MemWrite, 4, this->IO_we);	   // Memory Control
        uint32_t alu_opcode = aluc.getALUoperation(opcode, funct7, funct3); // ALU Control

        /********************************************
         ******************EXECUTE*******************
        *******************************************/
        // First ALU operand multiplexor
        uint32_t A;
        switch (PC_select) {
        case 0:
            A = rf.read_rs1();
            break;
        case 1:
            A = pc_addr; // jal, jalr, b-type
            break;
        default:
            A = 0; // lui
            break;
        }

        uint32_t B;
        // Second ALU operand multiplexor
        switch (ALUSrc) {
        case 0: // +4
            B = 4;
            break;
        case 1: // Register Source 2
            B = rf.read_rs2();
            break;
        default: // Immediate
            B = immediate;
            break;
        }

        // Execute operation on operands
        uint32_t alu_output = alu.exec(A, B, alu_opcode);

        /********************************************
         ****************WRITE-BACK******************
        *******************************************/
        // Register File Multiplexor
        uint32_t data_out = dram.read_data(alu_output); // Data Memory at ALU computed address
        uint32_t csr_rd = csr.get_csr(csr_addr);		// Register read from CSR

        // Write-back data
        uint32_t rf_data_writeback;
        if (MemToReg == 0) {
            rf_data_writeback = alu_output; // Write ALU output
        } else if (MemToReg == 1) {
            rf_data_writeback = data_out; // Write Data Memory
        } else {
            rf_data_writeback = csr_rd; // Write CSR register
        }

        rf.write(rf_data_writeback, rd);			// Write to Register File if applicable
        dram.write_data(alu_output, rf.read_rs2()); // Write to Data Memory if applicable
        csr.update_csr(csr_addr, rf.read_rs1());	// Write to CSR if applicable

        // Update PC
        nextpc.calculateNextPC(immediate, opcode, funct3, A, B, csr.get_csr(0x003), trap_taken);
        pc.setPC(nextpc.getNextPC());

        // debug_post_execute(opcode, rd, immediate);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        std::cerr << "Invalid Arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    


    // Memory
    std::multimap<uint32_t, uint32_t> dram_flash;

    // Flash text section to Memory
    std::ifstream text_input(argv[1], std::ios::binary);
    std::vector<char> text_section(
         (std::istreambuf_iterator<char>(text_input)),
         (std::istreambuf_iterator<char>()));
    text_input.close();
    int num_instructions = text_section.size() / 4;    
    for (int i = 0; i < num_instructions; i++) {
        // Preliminary 32-bit instruction
        uint32_t instruction = ((text_section[4*i+3] << 24) & 0xff000000) |
                               ((text_section[4*i+2] << 16) & 0x00ff0000) |
                               ((text_section[4*i+1] << 8)  & 0x0000ff00) |
                               ((text_section[4*i+0])       & 0x000000ff);
        dram_flash.insert(std::make_pair((uint32_t)(ROM_START + 4*i), instruction));
    }

    // Flash data segment
    std::ifstream data_input(argv[2], std::ios::binary);
    std::vector<char> data_section(
         (std::istreambuf_iterator<char>(data_input)),
         (std::istreambuf_iterator<char>()));
    data_input.close();
    int num_data = data_section.size() / 4;    
    for (int i = 0; i < num_data; i++) {
        // Preliminary 32-bit instruction
        uint32_t data = ((data_section[4*i+3] << 24) & 0xff000000) |
                        ((data_section[4*i+2] << 16) & 0x00ff0000) |
                        ((data_section[4*i+1] << 8)  & 0x0000ff00) |
                        ((data_section[4*i+0])       & 0x000000ff);
        dram_flash.insert(std::make_pair((uint32_t)(RAM_START + 4*i), data));
    }

    CPU t89(dram_flash);
    while(1) {
        t89.executeInstruction();
    }
}
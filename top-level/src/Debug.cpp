#include <iostream>
#include "Pipeline.h"

void Debug::debug_pre_execute(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t rs1, uint32_t rs2, uint32_t rd, uint32_t immediate, uint32_t csr_addr, uint32_t cur_instruction)
{
	std::cout << opcode << std::endl;
	std::cout << "Current Instruction: " << cur_instruction << std::endl;
	switch (opcode) {
	case LUI: std::cout << "immediate: " << immediate << " rd: " << rd << " opcode: " << opcode << std::endl; break;
	case ITYPE: std::cout << "immediate: " << immediate << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << " opcode: " << opcode << std::endl; break;
	case RTYPE: std::cout << "rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << " opcode: " << opcode << std::endl; break;
	case LOAD: std::cout << "immediate: " << immediate << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << " opcode: " << opcode << std::endl; break;
	case STORE: std::cout << "immediate: " << immediate << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " opcode: " << opcode << std::endl; break;
	case BTYPE: std::cout << "immediate: " << immediate << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " opcode: " << opcode << std::endl; break;
	case JAL: std::cout << "immediate: " << immediate << " rd: " << rd << " opcode: " << opcode << std::endl; break;
	case AUIPC: std::cout << "immediate: " << immediate << " rd: " << rd << " opcode: " << opcode << std::endl; break;
	case JALR: std::cout << "immediate: " << immediate << " rd: " << rd << " rs: " << rs1 << " opcode: " << opcode << std::endl; break;
	case ECALL: // ecall/csr
		if (funct3 == 0) {
			std::cout << "ecall" << std::endl;
		}
		break;
	}
}

void Debug::debug_post_execute(uint32_t opcode, uint32_t rd, uint32_t immediate, uint32_t rd_data, uint32_t rs2_data, uint32_t rs1_data, uint32_t pc_addr)
{
	switch (opcode) {
		case LUI: std::cout << "Wrote " << rd_data << " to register " << rd << std::endl << std::endl; break;
		case ITYPE: std::cout << "Wrote " << rd_data << " to register " << rd << std::endl << std::endl; break;
		case RTYPE: std::cout << "Wrote " << rd_data << " to register " << rd << std::endl << std::endl; break;
		case LOAD: std::cout << "Wrote " << rd_data << " to register " << rd << std::endl << std::endl; break;
		case STORE: std::cout << "Wrote " << rs2_data << " to address " << rs1_data + immediate << std::endl << std::endl; break;
		case BTYPE: std::cout << "Next PC: " << pc_addr << std::endl << std::endl; break;
		case JAL: std::cout << "Next PC: " << pc_addr << ". Wrote " << rd_data << " to register " << rd << std::endl << std::endl; break;
		case AUIPC: std::cout << "Wrote " << pc_addr - 4 + immediate << " to register " << rd << std::endl << std::endl; break;
		case JALR: std::cout << "Next PC: " << pc_addr << ". Wrote " << rd_data << " to register " << rd << std::endl << std::endl; break;
		case ECALL: std::cout << "Jump to handler at: " << pc_addr << std::endl << std::endl; break;
	}
}

class Debug;
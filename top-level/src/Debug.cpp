#include <iostream>
#include "Pipeline.h"

void Debug::debug_pre_execute(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t rs1, uint32_t rs2, uint32_t rd, uint32_t immediate, uint32_t csr_addr, uint32_t cur_instruction)
{
	std::cout << opcode << std::endl;
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
		if (funct3 == 0) {
			std::cout << "ecall" << std::endl;
		}
		break;
	}
}

void Debug::debug_post_execute(uint32_t opcode, uint32_t rd, uint32_t immediate)
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
		std::cout << "Next PC: " << pc.getPC() << std::endl << std::endl;
		break;
	case 0b1101111: // jal
		std::cout << "Next PC: " << pc.getPC() << ". Wrote " << rf.read_rd() << " to register " << rd << std::endl << std::endl;
		break;
	case 0b0010111: // auipc
		std::cout << "Wrote " << pc.getPC() - 4 + immediate << " to register " << rd << std::endl << std::endl;
		break;
	case 0b1100111: // jalr
		std::cout << "Next PC: " << pc.getPC() << ". Wrote " << rf.read_rd() << " to register " << rd << std::endl << std::endl;
		break;
	case 0b1110011: // ecall/csr
		std::cout << "mcause: " << csr.get_csr(4) << std::endl;
		std::cout << "mepc: " << csr.get_csr(2) << std::endl;
		std::cout << "Jump to handler at: " << pc.getPC() << std::endl << std::endl;
		break;
	}
}

class Debug;
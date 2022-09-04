#include "CPU.h"

void CPU::debug_pre_execute(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t rs1, uint32_t rs2, uint32_t rd, uint32_t immediate, uint32_t csr_addr, uint32_t cur_instruction)
{
	if(cur_instruction == 0) {exit(1);}
	std::cout << std::hex << "Current Instruction: " << cur_instruction << std::endl;
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
	case PRIV: // ecall/csr
		if (funct3 == 0) {
			std::cout << "ecall" << std::endl;
		}
		break;
	}
}

void CPU::debug_post_execute(uint32_t opcode, uint32_t rd, uint32_t immediate, uint32_t rd_data, uint32_t rs2_data, uint32_t rs1_data, uint32_t pc_addr)
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
		case PRIV: std::cout << "Jump to handler at: " << pc_addr << std::endl << std::endl; break;
	}
}

CPU::CPU(uint32_t rom_base, uint32_t rom_size, uint32_t ram_base, uint32_t ram_size, int debug = 0) {
	rf = new RegisterFile;
	pc = new ProgramCounter;
	csr = new CSR;
	alu = new ALU;
	aluc = new ALUControlUnit;
	immgen = new ImmediateGenerator;
	mcu = new MemControlUnit;
	nextpc = new NextPC;
	bus = new Bus(rom_base, rom_size, ram_base, ram_size);
	trap = new Trap;
}

CPU::~CPU() {
	delete rf;
	delete pc;
	delete bus;
	delete csr;
	delete alu;
	delete aluc;
	delete immgen;
	delete mcu;
	delete nextpc;
	delete trap;
}


void CPU::next_instruction()
{
	// Update Clint Device every cycle
	bus->clint_device->next_cycle(csr);

	// Check for interrupts
	if (bus->clint_device->check_interrupts(csr)) {
		trap->take_trap(csr, pc, nextpc, bus->clint_device->interrupt_type);
	}

	execute_instruction();
}

bool CPU::execute_instruction() {
	uint32_t trap_taken = 0;

	// Fetch Stage
	uint32_t pc_addr = pc->getPC();						 // Current PC
	uint32_t cur_instruction = bus->read(pc_addr, WORD); // Current Instruction
	
	// Decode Stage
	uint32_t opcode = cur_instruction & 0b1111111;			   // opcode field
	uint32_t funct3 = (cur_instruction >> 12) & 0b111;		   // funct3 field
	uint32_t funct7 = (cur_instruction >> 25) & 0b1111111;	   // funct7 field
	uint32_t rs1 = (cur_instruction >> 15) & 0b11111;		   // Register Source 1
	uint32_t rs2 = (cur_instruction >> 20) & 0b11111;		   // Register Source 2
	uint32_t rd = (cur_instruction >> 7) & 0b11111;			   // Register Desination
	uint32_t immediate = immgen->getImmediate(cur_instruction); // Instruction Immediate
	uint32_t csr_addr = (cur_instruction >> 20) & 0xfff;	   // CSR Address

	// Execution flow dependent on instrution type
	uint32_t mem_size;
	uint32_t A = 0;
	uint32_t B = 0;
	uint32_t alu_opcode;
	uint32_t alu_output;
	uint32_t cause_offset = 0;
	
	switch(opcode) {
	case LUI:
		rf->write(immediate, rd);
		break;
	case AUIPC:
		alu_opcode = aluc->getALUoperation(opcode, 0, 0);
		alu_output = alu->execute(pc_addr, immediate, alu_opcode);
		rf->write(alu_output, rd);
		break;
	case JAL:
		alu_opcode = aluc->getALUoperation(opcode, 0, 0);
		alu_output = alu->execute(pc_addr, 4, alu_opcode);
		rf->write(alu_output, rd);
		break;
	case JALR:
		A = rf->read(rs1);
		alu_opcode = aluc->getALUoperation(opcode, 0, 0);
		alu_output = alu->execute(pc_addr, 4, alu_opcode);
		rf->write(alu_output, rd);
		break;
	case BTYPE:
		A = rf->read(rs1);
		B = rf->read(rs2);
		break;
	case LOAD:
		mem_size = mcu->get_mem_size(funct3);
		rf->write(bus->read(rf->read(rs1) + immediate, mem_size), rd);
		break;
	case STORE:
		mem_size = mcu->get_mem_size(funct3);
		bus->write(rf->read(rs1) + immediate, rf->read(rs2), mem_size);
		break;
	case ITYPE:
		alu_opcode = aluc->getALUoperation(opcode, funct7, funct3);
		alu_output = alu->execute(rf->read(rs1), immediate, alu_opcode);
		rf->write(alu_output, rd);
		break;
	case RTYPE:
		alu_opcode = aluc->getALUoperation(opcode, funct7, funct3);
		alu_output = alu->execute(rf->read(rs1), rf->read(rs2), alu_opcode);
		rf->write(alu_output, rd);
		break;
	case PRIV:
		switch (funct3) {
		case 0b000: // ECALL / MRET
			switch (immediate) {
			case MRET_IMM:
				// MRET Instruction
				// Restore MIE field from MPIE
				if (csr->get_mpie()) { // Set MIE if MPIE is 1
					csr->set_mie();
				}
				// Set MPIE to 1
				csr->set_mpie();
				// Set MPP to privilege mode before trap
				csr->set_mpp(MACHINE_MODE);
				break;
			case ECALL_IMM:
				csr->set_msip();
				if (csr->get_mie() && csr->get_msie() && csr->get_msip()) {
					cause_offset = 3;
					// Save mepc
					csr->mepc = pc->PC + WORD;
					// Save MIE to MPIE before trap
					if (csr->get_mie()) // MIE Enabled, save to MPIE
						csr->set_mpie();
					csr->reset_mie();
					csr->set_mpp(MACHINE_MODE);
					trap_taken = 1;
				}
				break;
			}
			break;
		case 0b001:									 // CSRRW
			rf->write(csr->read_csr(csr_addr), rd);	 // Write current value of CSR to rd
			csr->write_csr(csr_addr, rf->read(rs1)); // Store value of rs1 into CSR
			break;
		case 0b010:									// CSRRS
			rf->write(csr->read_csr(csr_addr), rd); // Write current value of CSR to rd
			if (rs1 != 0)
				csr->write_csr(csr_addr, rf->read(rs1) | csr->read_csr(csr_addr)); // Use rs1 as a bit mask to set CSR bits
			break;
		case 0b011:									// CSRRC
			rf->write(csr->read_csr(csr_addr), rd); // Write current value of CSR to rd
			if (rs1 != 0)
				csr->write_csr(csr_addr, (!rf->read(rs1)) & csr->read_csr(csr_addr)); // Usr rs1 as a bit mask to reset CSR bits
			break;
		default:
			// Immediate CSR Instructions not yet supported
			break;
		}
		break;
	default:
		// Illegal Instruction
		break;
	}

	// Update PC
	pc->setPC(nextpc->calculateNextPC(immediate, opcode, funct3, A, B, csr->mtvec + 4*cause_offset, trap_taken, csr->mepc));
	return true;
}
#define OLC_PGE_APPLICATION
#include "Pipeline.h"

void Pipeline::debug_pre_execute(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t rs1, uint32_t rs2, uint32_t rd, uint32_t immediate, uint32_t csr_addr, uint32_t cur_instruction)
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

void Pipeline::debug_post_execute(uint32_t opcode, uint32_t rd, uint32_t immediate, uint32_t rd_data, uint32_t rs2_data, uint32_t rs1_data, uint32_t pc_addr)
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

void Pipeline::query_external_interrupt()
{
	// Handle User Input (External Hardware Interrupt)
	if (GetKey(olc::Key::TAB).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::TAB;
		this->IO_ADDR = 0x90000000;
		this->interrupt_assert = 0;
	}
}

Pipeline::Pipeline(std::multimap<uint32_t, uint32_t> memory, int debug = 0)
{
	sAppName = "VGA Output";
	this->interrupt_assert = 0;
	this->IO_BUS = 0;	 // 32 bit IO bus
	this->IO_ADDR = 0;   // 32-bit IO address
	this->debug_mode = debug; // Debug mode

	// Load Memory
	for (std::multimap<uint32_t, uint32_t>::iterator it = memory.begin(); it != memory.end(); it++)
	{
		dram.write(it->first, it->second, WORD);
	}
}

bool Pipeline::OnUserCreate() { return true; }

bool Pipeline::OnUserUpdate(float fElapsedTime)
{
	uint32_t trap_taken = 0;

	// Micro-Controller
	this->query_external_interrupt(); // Check for external interrupt assertion
	if ((csr.read_csr(MIE) == 0x888) && (this->interrupt_assert == 1)) {
		// External Interrupt and MIE enabled
		dram.write(this->IO_ADDR, this->IO_BUS, WORD); // write IO data to IO port
		this->interrupt_assert = 0;
		trap_taken = 1;
	}

	if (this->debug_mode && (this->IO_BUS != (uint32_t)olc::Key::TAB))
	{
		return true; // Debug Mode
	}

	// Fetch Stage
	uint32_t pc_addr = pc.getPC();						 // Current PC
	uint32_t cur_instruction = dram.read(pc_addr, WORD); // Current Instruction

	// Decode Stage
	uint32_t opcode = cur_instruction & 0b1111111;			   // opcode field
	uint32_t funct3 = (cur_instruction >> 12) & 0b111;		   // funct3 field
	uint32_t funct7 = (cur_instruction >> 25) & 0b1111111;	   // funct7 field
	uint32_t rs1 = (cur_instruction >> 15) & 0b11111;		   // Register Source 1
	uint32_t rs2 = (cur_instruction >> 20) & 0b11111;		   // Register Source 2
	uint32_t rd = (cur_instruction >> 7) & 0b11111;			   // Register Desination
	uint32_t immediate = immgen.getImmediate(cur_instruction); // Instruction Immediate
	uint32_t csr_addr = (cur_instruction >> 20) & 0xfff;	   // CSR Address

	/******************DEBUG********************/
	if (this->debug_mode)
	{
		this->debug_pre_execute(opcode, funct3, funct7, rs1, rs2, rd, immediate, csr_addr, cur_instruction);
	}
	/******************DEBUG********************/

	// Execution flow dependent on instrution type
	uint32_t mem_size;
	uint32_t A = 0;
	uint32_t B = 0;
	uint32_t alu_opcode;
	uint32_t alu_output;

	switch(opcode) {
		case LUI:
			rf.write(immediate, rd);
			break;
		case AUIPC:
			alu_opcode = aluc.getALUoperation(opcode, 0, 0);
			alu_output = alu.execute(pc_addr, immediate, alu_opcode);
			rf.write(alu_output, rd);
			break;
		case JAL:
			alu_opcode = aluc.getALUoperation(opcode, 0, 0);
			alu_output = alu.execute(pc_addr, 4, alu_opcode);
			rf.write(alu_output, rd);
			break;
		case JALR:
			alu_opcode = aluc.getALUoperation(opcode, 0, 0);
			alu_output = alu.execute(pc_addr, 4, alu_opcode);
			rf.write(alu_output, rd);
			break;
		case BTYPE:
			A = rf.read(rs1);
			B = rf.read(rs2);
			break;
		case LOAD:
			mem_size = mcu.get_mem_size(funct3);
			rf.write(dram.read(rf.read(rs1) + immediate, mem_size), rd);
			break;
		case STORE:
			mem_size = mcu.get_mem_size(funct3);
			dram.write(rf.read(rs1) + immediate, rf.read(rs2), mem_size);
			break;
		case ITYPE:
			alu_opcode = aluc.getALUoperation(opcode, funct7, funct3);
			alu_output = alu.execute(rf.read(rs1), immediate, alu_opcode);
			rf.write(alu_output, rd);
			break;
		case RTYPE:
			alu_opcode = aluc.getALUoperation(opcode, funct7, funct3);
			alu_output = alu.execute(rf.read(rs1), rf.read(rs2), alu_opcode);
			rf.write(alu_output, rd);
			break;
		case ECALL:
			funct3 = (cur_instruction >> 12) & 0b111;
			switch(funct3) {
				case 0b000: // ECALL
					trap_taken = 1;
					break;
				default: // CSR instruction
					// Later
					break;
			}
			break;
	}
	
	// Update PC
	pc.setPC(nextpc.calculateNextPC(immediate, opcode, funct3, A, B, csr.read_csr(MTVEC), trap_taken));

	/******************DEBUG********************/
	if (this->debug_mode)
	{
		this->debug_post_execute(opcode, rd, immediate, rf.read(rd), rf.read(rs2), rf.read(rs1), pc.getPC());
	}
	/******************DEBUG********************/

	// Update VRAM
	uint32_t pixel;
	uint32_t pixel_addr;
	uint32_t x;
	uint32_t y;
	if ((pixel_addr = dram.get_changed_pixel()) != 0) {
		pixel = dram.read(pixel_addr, WORD);
		pixel_addr -= VRAM_START;
		x = (pixel_addr/4) % WIDTH;
		y = (pixel_addr/4) / WIDTH;
		uint8_t red = (pixel >> 24) & 0xff;
		uint8_t green = (pixel >> 16) & 0xff;
		uint8_t blue = (pixel >> 8) & 0xff;
		Draw(x, y, olc::Pixel(red, green, blue));
	}
	return true;
}
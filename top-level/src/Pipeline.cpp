#include "Pipeline.h"

void CPU::query_external_interrupt()
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
	this->debug = debug; // Debug mode

	// Load Memory
	for (std::multimap<uint32_t, uint32_t>::iterator it = memory.begin(); it != memory.end(); it++)
	{
		dram.write(it->first, it->second, WORD);
	}
}

bool Pipeline::OnUserCreate() { return true; }

bool Pipeline::OnUserUpdate(float fElapsedTime)
{
	/********************************************
	 * ************Microcontroller***************
	 * *****************************************/
	debug.query_external_interrupt(); // Check for external interrupt assertion
	if ((csr.read(MIE) == 0x888) && (this->interrupt_assert == 1)) {
		// External Interrupt and MIE enabled
		dram.write_io(this->IO_addr, this->IO_BUS, WORD); // write IO data to IO port
		this->interrupt_assert = 0;
	}

	if (this->debug && (this->IO_BUS != (uint32_t)olc::Key::TAB))
	{
		return true; // Debug Mode
	}

	/********************************************
	 *******************FETCH********************
	 *******************************************/
	uint32_t pc_addr = pc.getPC();						 // Current PC
	uint32_t cur_instruction = dram.read(pc_addr, WORD); // Current Instruction

	/********************************************
	 *******************DECODE*******************
	 *******************************************/
	uint32_t opcode = cur_instruction & 0b1111111;			   // opcode field
	uint32_t funct3 = (cur_instruction >> 12) & 0b111;		   // funct3 field
	uint32_t funct7 = (cur_instruction >> 25) & 0b1111111;	   // funct7 field
	uint32_t rs1 = (cur_instruction >> 15) & 0b11111;		   // Register Source 1
	uint32_t rs2 = (cur_instruction >> 20) & 0b11111;		   // Register Source 2
	uint32_t rd = (cur_instruction >> 7) & 0b11111;			   // Register Desination
	uint32_t immediate = immgen.getImmediate(cur_instruction); // Instruction Immediate
	uint32_t csr_addr = (cur_instruction >> 20) & 0xfff;	   // CSR Address

	/******************DEBUG********************/
	if (this->debug)
	{
		debug_pre_execute(opcode, funct3, funct7, rs1, rs2, rd, immediate, csr_addr, cur_instruction);
	}
	/******************DEBUG********************/

	
	// Execution flow dependent on instrution type
	uint32_t A;
	uint32_t B;
	uint32_t alu_opcode;
	uint32_t mem_size;

	switch(opcode) {
		case LUI:
			rd = (cur_instruction >> 7) & 0b11111
			immediate = immgen.getImmediate(cur_instruction);
			rf.write(immediate, rd);
			break;
		case AUIPC:
			
	}
	
	uint32_t alu_opcode = aluc.getALUoperation(opcode, funct7, funct3); // ALU Control

	/********************************************
	 ******************EXECUTE*******************
	 *******************************************/
	// First ALU operand multiplexor
	uint32_t A;
	if (PC_select == 0) {
		A = rf.read_rs1();
	} else if (PC_select == 1) {
		A = pc_addr; // jal, jalr, b-type
	} else {
		A = 0; // lui
	}

	uint32_t B;
	// Second ALU operand multiplexor
	if (ALUSrc == 0)
		B = 4; // +4
	else if (ALUSrc == 1)
		B = rf.read_rs2(); // Register Source 2
	else
		B = immediate; // Immediate

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

	/******************DEBUG********************/
	if (this->debug)
	{
		debug_post_execute(opcode, rd, immediate);
	}
	/******************DEBUG********************/

	// Update VRAM
	uint32_t pixel;
	uint32_t pixel_addr;
	uint32_t x;
	uint32_t y;
	if ((pixel_addr = dram.get_changed_pixel()) != 0) {
		pixel = dram.read_data(pixel_addr);
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
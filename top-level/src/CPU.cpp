#define OLC_PGE_APPLICATION
// #include "olcPixelGameEngine.h"
#include "Components.h"
#include "CPU.h"

uint32_t IO_BUS;  // 32 bit IO bus
uint32_t IO_we;	  // IO write enable
uint32_t IO_addr; // Address of device
int interrupt_assert;
int debug;

void CPU::debug_pre_execute(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t rs1, uint32_t rs2, uint32_t rd, uint32_t immediate, uint32_t csr_addr, uint32_t cur_instruction)
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
		if (funct3 == 0) {
			std::cout << "ecall" << std::endl;
		}
		break;
	}
}

void CPU::debug_post_execute(uint32_t opcode, uint32_t rd, uint32_t immediate)
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

void CPU::query_external_interrupt()
{
	// Handle User Input (to optimize)
	this->IO_BUS = 0;
	if (GetKey(olc::Key::LEFT).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::LEFT;
		this->interrupt_assert = 1;
		this->IO_addr = 0;
		this->IO_we = 1;
	}
	else if (GetKey(olc::Key::RIGHT).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::RIGHT;
		this->interrupt_assert = 1;
		this->IO_addr = 0;
		this->IO_we = 1;
	}
	else if (GetKey(olc::Key::A).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::A;
		this->interrupt_assert = 1;
		this->IO_addr = 0;
		this->IO_we = 1;
	}
	else if (GetKey(olc::Key::W).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::W;
		this->interrupt_assert = 1;
		this->IO_addr = 0;
		this->IO_we = 1;
	}
	else if (GetKey(olc::Key::S).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::S;
		this->interrupt_assert = 1;
		this->IO_addr = 0;
		this->IO_we = 1;
	}
	else if (GetKey(olc::Key::D).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::D;
		this->interrupt_assert = 1;
		this->IO_addr = 0;
		this->IO_we = 1;
	}
	else if (GetKey(olc::Key::TAB).bPressed)
	{
		this->IO_BUS = (uint32_t)olc::Key::TAB;
		this->interrupt_assert = 0;
		this->IO_addr = 0;
		this->IO_we = 0;
	}
}

CPU::CPU(std::multimap<uint32_t, uint32_t> memory, int debug = 0)
{
	sAppName = "VGA Output";
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

bool CPU::OnUserCreate() { return true; }
bool CPU::OnUserUpdate(float fElapsedTime)
{
	/********************************************
	 * ************Microcontroller***************
	 * *****************************************/
	int external_interrupt = 0;
	query_external_interrupt(); // Check for external interrupt assertion
	if ((this->IO_we == 1) && (csr.get_csr(0x000) == 1))
	{
		// External Interrupt and MIE enabled
		dram.set_control_signals(0, 0, 4, this->IO_we);
		dram.write_io(this->IO_addr, this->IO_BUS); // write IO data to IO port
		external_interrupt = 1;
		this->IO_we = 0;
		this->interrupt_assert = 0;
	}

	if (this->debug && (this->IO_BUS != (uint32_t)olc::Key::TAB))
	{
		return true; // Debug Mode
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

	/******************DEBUG********************/
	if (this->debug)
	{
		debug_pre_execute(opcode, funct3, funct7, rs1, rs2, rd, immediate, csr_addr, cur_instruction);
	}
	/******************DEBUG********************/

	// Control Unit Signals
	ctrl.setControlLines(opcode, external_interrupt, funct3, mode);
	uint32_t RegWrite = ctrl.get_RegWrite();   // Write to register?
	uint32_t ALUSrc = ctrl.get_ALUSrc();	   // What to feed to ALU?
	uint32_t MemToReg = ctrl.get_MemtoReg();   // Read from data memory?
	uint32_t MemRead = ctrl.get_MemRead();	   // Which value to put in Register File?
	uint32_t MemWrite = ctrl.get_MemWrite();   // Write to memory?
	uint32_t ALUop = ctrl.get_ALUop();		   // ALU operation
	uint32_t PC_select = ctrl.get_PC_Select(); // Select PC or rs1
	uint32_t trap_taken = ctrl.get_trap();	   // Take a trap?
	uint32_t mcause = ctrl.get_mcause();	   // Machine cause of trap
	uint32_t CSR_en = ctrl.get_CSR_enable();   // Write to CSR module?

	// Control Lines - CSR, Register File, ALU
	csr.set_control_lines(CSR_en, trap_taken, mcause, pc_addr);		   // CSR Control
	rf.set_control_lines(rs1, rs2, RegWrite);						   // Register File Control
	dram.set_control_signals(MemRead, MemWrite, 4, this->IO_we);	   // Memory Control
	uint32_t alu_opcode = aluc.getALUoperation(ALUop, funct7, funct3); // ALU Control

	/********************************************
	 ******************EXECUTE*******************
	 *******************************************/
	// First ALU operand multiplexor
	uint32_t A = (PC_select == 1) ? pc_addr : rf.read_rs1();
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
	if (MemToReg == 0)
		rf_data_writeback = alu_output; // Write ALU output
	else if (MemToReg == 1)
		rf_data_writeback = data_out; // Write Data Memory
	else
		rf_data_writeback = csr_rd; // Write CSR register

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

	return true;
}

class CPU;
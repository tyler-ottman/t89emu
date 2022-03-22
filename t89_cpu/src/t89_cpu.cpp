#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "../../t89_cpu_components/include/Components.h"
#include <map>

class CPU : public olc::PixelGameEngine
{
private:
	uint32_t IO_BUS;	// 32 bit IO bus
	uint32_t IO_we;		// IO write enable
	uint32_t IO_addr;	// Address of device
	int interrupt_assert;
	int debug;

	ALU<uint32_t> alu;
	ALUControlUnit aluc;
	ControlUnit ctrl;
	CSR csr;
	ImmediateGenerator<uint32_t> immgen;
	Memory dram;
	NextPC<uint32_t> nextpc;
	ProgramCounter pc;
	RegisterFile<uint32_t> rf;

	void query_external_interrupt() {
		// Handle User Input
		this->IO_BUS = 0;
		if (GetKey(olc::Key::LEFT).bPressed){
			this->IO_BUS = (uint32_t)olc::Key::LEFT;
			this->interrupt_assert = 1;
			this->IO_addr = 0;
			this->IO_we = 1;
		} else if (GetKey(olc::Key::RIGHT).bPressed){
			this->IO_BUS = (uint32_t)olc::Key::RIGHT;
			this->interrupt_assert = 1;
			this->IO_addr = 0;
			this->IO_we = 1;
		} else if (GetKey(olc::Key::A).bPressed){
			this->IO_BUS = (uint32_t)olc::Key::A;
			this->interrupt_assert = 1;
			this->IO_addr = 0;
			this->IO_we = 1;
		} else if (GetKey(olc::Key::W).bPressed){
			this->IO_BUS = (uint32_t)olc::Key::W;
			this->interrupt_assert = 1;
			this->IO_addr = 0;
			this->IO_we = 1;
		} else if (GetKey(olc::Key::S).bPressed){
			this->IO_BUS = (uint32_t)olc::Key::S;
			this->interrupt_assert = 1;
			this->IO_addr = 0;
			this->IO_we = 1;
		} else if (GetKey(olc::Key::D).bPressed){
			this->IO_BUS = (uint32_t)olc::Key::D;
			this->interrupt_assert = 1;
			this->IO_addr = 0;
			this->IO_we = 1;
		} else if (GetKey(olc::Key::TAB).bPressed){
			this->IO_BUS = (uint32_t)olc::Key::TAB;
			this->interrupt_assert = 0;
			this->IO_addr = 0;
			this->IO_we = 0;
		}
	}
public:
	CPU(std::multimap<uint32_t, uint32_t> memory, int debug = 0) {
		sAppName = "VGA Output";
		this->interrupt_assert = 0;
		this->IO_BUS = 0;		// 32 bit IO bus
		this->IO_we = 0;		// IO write enable
		this->IO_addr = 0;		// Address of device
		this->debug = debug;	// Debug mode
		
		// Load Memory
		dram.set_control_signals(1, 1, 4, 1);
		for (std::multimap<uint32_t, uint32_t>::iterator it = memory.begin(); it != memory.end(); it++) {
			dram.write_data(it->first, it->second);
		}
	}
	bool OnUserCreate() override {return true;}
	bool OnUserUpdate(float fElapsedTime) override
	{	
		/********************************************
		 * ************Microcontroller***************
		 * *****************************************/	
		int external_interrupt = 0;	
		query_external_interrupt();											// Check for external interrupt assertion
		if ((this->IO_we == 1) && (csr.get_csr(0x000) == 1)) {
			// External Interrupt and MIE enabled
			dram.set_control_signals(0, 0, 4, this->IO_we);
			dram.write_io(this->IO_addr, this->IO_BUS);						// write IO data to IO port
			external_interrupt = 1;
			this->IO_we = 0;
			this->interrupt_assert = 0;
		}

		if(this->debug && (this->IO_BUS != (uint32_t)olc::Key::TAB)) {
			return true;													// Debug Mode
		}

		/********************************************
		 *******************FETCH********************
		 *******************************************/
		uint32_t pc_addr = pc.getPC();										// Current PC
		uint32_t cur_instruction = dram.read_data(pc_addr);					// Current Instruction

		/********************************************
		 *******************DECODE*******************
		 *******************************************/
		uint32_t opcode = cur_instruction & 0b1111111;						// opcode field
		uint32_t funct3 = (cur_instruction >> 12) & 0b111;					// funct3 field
		uint32_t funct7 = (cur_instruction >> 25) & 0b1111111;				// funct7 field
		uint32_t mode = csr.get_csr(0x005);									// mode field
		uint32_t rs1 = (cur_instruction >> 15) & 0b11111;					// Register Source 1
		uint32_t rs2 = (cur_instruction >> 20) & 0b11111;					// Register Source 2
		uint32_t rd  = (cur_instruction >> 7)  & 0b11111;					// Register Desination
		uint32_t immediate = immgen.getImmediate(cur_instruction);			// Instruction Immediate
		uint32_t csr_addr = (cur_instruction >> 20) & 0xfff;				// CSR Address
		
		/******************DEBUG********************/
		if (this->debug) {
			std::cout << "Current Instruction: " << cur_instruction << std::endl;
			switch(opcode) {
				case 0b0110111:												// lui
					std::cout << "immediate: " << immediate;
					std::cout << " rd: " << rd;
					std::cout << " opcode: " << opcode << std::endl;
					break;
				case 0b0010011:												// I-type
					std::cout << "immediate: " << immediate;
					std::cout << " rs1: " << rs1;
					std::cout << " funct3: " << funct3;
					std::cout << " rd: " << rd;
					std::cout << " opcode: " << opcode << std::endl;
					break;
				case 0b0110011:												// R-type
					std::cout << "rs2: " << rs2;
					std::cout << " rs1: " << rs1;
					std::cout << " funct3: " << funct3;
					std::cout << " rd: " << rd;
					std::cout << " opcode: " << opcode << std::endl;
					break;
			}
		}
		/******************DEBUG********************/

		// Control Unit Signals
		ctrl.setControlLines(opcode, external_interrupt, funct3, mode);
		uint32_t RegWrite = ctrl.get_RegWrite();							// Write to register?
		uint32_t ALUSrc = ctrl.get_ALUSrc();								// What to feed to ALU?
		uint32_t MemToReg = ctrl.get_MemtoReg();							// Read from data memory?
		uint32_t MemRead = ctrl.get_MemRead();								// Which value to put in Register File?
		uint32_t MemWrite = ctrl.get_MemWrite();							// Write to memory?
		uint32_t ALUop = ctrl.get_ALUop();									// ALU operation
		uint32_t PC_select = ctrl.get_PC_Select();							// Select PC or rs1
		uint32_t trap_taken = ctrl.get_trap();								// Take a trap?
		uint32_t mcause = ctrl.get_mcause();								// Machine cause of trap
		uint32_t CSR_en = ctrl.get_CSR_enable();							// Write to CSR module?

		// Control Lines - CSR, Register File, ALU
		csr.set_control_lines(CSR_en, trap_taken, mcause, pc_addr);			// CSR Control
		rf.set_control_lines(rs1, rs2, RegWrite);							// Register File Control
		dram.set_control_signals(MemRead, MemWrite, 4, this->IO_we);		// Memory Control
		uint32_t alu_opcode = aluc.getALUoperation(ALUop, funct7, funct3);	// ALU Control

		/********************************************
		 ******************EXECUTE*******************
		 *******************************************/
		// First ALU operand multiplexor
		uint32_t A = (PC_select == 1) ? pc_addr : rf.read_rs1();
		uint32_t B;
		
		// Second ALU operand multiplexor
		if (ALUSrc == 0) B = 4; 											// +4
		else if (ALUSrc == 1) B = rf.read_rs2(); 							// Register Source 2
		else B = immediate; 												// Immediate

		// Execute operation on operands
		uint32_t alu_output = alu.exec(A, B, alu_opcode);

		/********************************************
		 ****************WRITE-BACK******************
		 *******************************************/
		// Register File Multiplexor
		uint32_t data_out = dram.read_data(alu_output);						// Data Memory at ALU computed address
		uint32_t csr_rd = csr.get_csr(csr_addr);							// Register read from CSR		

		// Write-back data
		uint32_t rf_data_writeback;
		if (MemToReg == 0) rf_data_writeback = alu_output;					// Write ALU output
		else if (MemToReg == 1) rf_data_writeback = data_out;				// Write Data Memory
		else rf_data_writeback = csr_rd;									// Write CSR register
		
		rf.write(rf_data_writeback, rd);									// Write to Register File if applicable
		dram.write_data(alu_output, rf.read_rs2());							// Write to Data Memory if applicable
		csr.update_csr(csr_addr, rf.read_rs1());							// Write to CSR if applicable

		// Update PC
		nextpc.calculateNextPC(immediate, opcode, funct3, A, B, csr.get_csr(0x003), trap_taken);
		pc.setPC(nextpc.getNextPC());

		// Debug, read registers
		if(this->debug) {
			switch(opcode) {
				case 0b0110111:												// lui
					std::cout << "Wrote " << rf.read_rd() << " to register " << rd << std::endl << std::endl;
					break;
				case 0b0010011: 											// I-Type
					std::cout << "Wrote " << rf.read_rd() << " to register " << rd << std::endl << std::endl;
					break;
				case 0b0110011:												// R-type
					std::cout << "Wrote " << rf.read_rd() << " to register " << rd << std::endl << std::endl;
					break;
			}
			// std::cout << "Register: Write " << rf.read_rd() << " to register " << rd << std::endl;
			// std::cout << "Memory written to: " << dram.read_data(alu_output) << std::endl;;
			// std::cout << "CSR written to: " << csr.get_csr(csr_addr) << std::endl;
			// std::cout << "Next PC: " << pc.getPC() << std::endl << std::endl;
		}
		return true;
	}
};

std::multimap<uint32_t, uint32_t> instructions = {
    {0x00000000, 0x00f58593},	// addi a1, a1, 0xf
    {0x00000004, 0x00f58593},	// addi a1, a1, 0xf
    {0x00000008, 0x0ff58613},	// addi a2, a1, 0xff
	{0x0000000c, 0xfff60613},	// addi a2, a2, 0xfff
	{0x00000010, 0xfff62613},	// slti a2, a1, 0xfff
	{0x00000014, 0x0ff5a593},	// slti a1, a1, 0xf (test signed later)
	{0x00000018, 0xfff5b613},	// sltiu a2, a1, 0xfff
	{0x0000001c, 0x0005b613},	// sltiu a2, a1, 0x0 (test "signed" later)
	{0x00000020, 0xabc64613},	// xori a2, a2, 0xabc
	{0x00000024, 0xabc64613},	// xori a2, a2, 0xabc
	{0x00000028, 0xaaa5e613},	// ori a2, a1, 0xaaa
	{0x0000002c, 0xfff66613},	// ori a2, a2, 0xfff
	{0x00000030, 0x0aa67613},	// andi a2, a2, 0x0aa
	{0x00000034, 0x05567613},	// andi a2, a2, 0x055
	{0x00000038, 0x00067613},	// andi a2, a2, 0x000
	{0x0000003c, 0xfff67613},	// andi a2, a2, 0xfff
	{0x00000040, 0x00359613},	// slli a2, a1, 0x003
	{0x00000044, 0x00258613},	// addi a2, a1, 0x002
	{0x00000048, 0x00361613},	// slli a2, a2, 0x003
	{0x0000004c, 0x00165613},	// srli a2, a2, 0x001
	{0x00000050, 0x80000637},	// lui a2, 0x80000
	{0x00000054, 0x40165613},	// srai a2, a2, 0x001
	{0x00000058, 0x40000637},	// lui a2, 0x40000
	{0x0000005c, 0x40165613},	// srai a2, a2, 0x001
	{0x00000060, 0x80000637},	// lui a2, 0x80000
	{0x00000064, 0x00162613},	// slti a2, a2, 0x001
	{0x00000068, 0xfffff637},	// lui a2, 0xfffff
	{0x0000006c, 0x00162613},	// slti a2, a2, 0x001
	{0x00000070, 0x80000637},	// lui a2, 0x80000
	{0x00000074, 0x00163613},	// sltiu a2, a2, 0x001
	{0x00000078, 0x00100593},	// addi a1, x0, 0x001
	{0x0000007c, 0x0025b613},	// sltiu a2, a1, 0x002
	{0x00000080, 0x00100013},	// addi x0, x0, 0x001
	{0x00000084, 0x00e00613},	// addi a2, x0, 0x00e
	{0x00000088, 0x00b60633},	// add a2, a2, a1
	{0x0000008c, 0x80000637},	// lui a2, 0x80000
	{0x00000090, 0x00b60633},	// add a2, a2, a1
	{0x00000094, 0x00800613},	// addi a2, x0, 0x008
	{0x00000098, 0x40b60633},	// sub a2, a2, a1
	{0x0000009c, 0x40c58633},	// sub a2, a1, a2
	{0x000000a0, 0x00b00613},	// addi a2, x0, 0x00b
	{0x000000a4, 0x00b61633},	// sll a2, a2, a1
	{0x000000a8, 0x00b61633},	// sll a2, a2, a1
	{0x000000ac, 0x00c5a633},	// slt a2, a1, a2
	{0x000000b0, 0x00062633}	// slt a2, a2, x0
};

int main(int argc, char* argv[])
{
	// Start simulation
	CPU t89(instructions, 1);
	if (t89.Construct(200, 200, 2, 2))
		t89.Start();
	
	exit(EXIT_SUCCESS);
}
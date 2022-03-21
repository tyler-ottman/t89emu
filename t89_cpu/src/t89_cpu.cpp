#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Components.h"

ALU<uint32_t> alu;
ALUControlUnit aluc;
ControlUnit ctrl;
CSR csr;
ImmediateGenerator<uint32_t> immgen;
Memory dram;
NextPC<uint32_t> nextpc;
ProgramCounter pc;
RegisterFile<uint32_t> rf;

uint32_t instr_index = 0;
#define NUM_INSTRUCTIONS 4
uint32_t instructions[NUM_INSTRUCTIONS] = {
	0x00f58593,		// addi a1, a1, 0xf
	0x00f58593,		// addi a1, a1, 0xf
	0x01e5b593,		// sltiu a1, 0x1e
	0xfff5c593		// xori a1, a1, 0xfff
};


class CPU : public olc::PixelGameEngine
{
private:
	uint32_t IO_BUS;	// 32 bit IO bus
	uint32_t IO_we;		// IO write enable
	uint32_t IO_addr;	// Address of device
	int interrupt_assert;

	void query_external_interrupt() {
		// Handle User Input
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
		}
	}
public:
	CPU() {
		sAppName = "VGA Output";
		this->interrupt_assert = 0;
	}
	bool OnUserCreate() override {return true;}
	bool OnUserUpdate(float fElapsedTime) override
	{	
		// Fetch instruction
		uint32_t pc_addr = pc.getPC();
		uint32_t cur_instruction = dram.read_data(pc_addr);
		
		// External interrupt
		query_external_interrupt();
		
		// If external interrupt
		if (this->IO_we) {
			// External interrupt taken
			// MemReadData, MemWriteData, size, IO_WR_enable
			dram.set_control_signals(0, 0, 4, this->IO_we);
			dram.write_io(this->IO_addr, this->IO_BUS);
		}

		// Decode instructions
		// Take interrupt if MIE enabled on interrupt assert signal is high
		int intr_taken = ((this->IO_we == 1) && (csr.get_csr(0x000) == 1)) ? 1 : 1;
		if (intr_taken) this->IO_we = 0;

		uint32_t opcode = cur_instruction & 0b1111111;
		uint32_t funct3 = (cur_instruction >> 12) & 0b111;
		uint32_t funct7 = (cur_instruction >> 25) & 0b1111111;
		// Get mode of CPU
		uint32_t mode = csr.get_csr(0x005);
		
		// opcode, interrupt, funct3, supervisor_mode
		ctrl.setControlLines(opcode, intr_taken, funct3, mode);

		uint32_t RegWrite = ctrl.get_RegWrite();		// Write to register?
		uint32_t ALUSrc = ctrl.get_ALUSrc();			// What to feed to ALU?
		uint32_t MemToReg = ctrl.get_MemtoReg();		// Read from data memory?
		uint32_t MemRead = ctrl.get_MemRead();			// Which value to put in Register File?
		uint32_t MemWrite = ctrl.get_MemWrite();		// Write to memory?
		uint32_t ALUop = ctrl.get_ALUop();				// ALU operation
		uint32_t PC_select = ctrl.get_PC_Select();		// Select PC or rs1
		uint32_t trap_taken = ctrl.get_trap();			// Take a trap?
		uint32_t mcause = ctrl.get_mcause();			// Machine cause of trap
		uint32_t CSR_en = ctrl.get_CSR_enable();		// Write to CSR module?

		// Set values for register file and immediate generator
		// Register source 1 of current instruction
		uint32_t rs1 = (cur_instruction >> 15) & 0b11111;
		uint32_t rs2 = (cur_instruction >> 20) & 0b11111;
		uint32_t rd  = (cur_instruction >> 7)  & 0b11111;
		rf.set_source_reg(rs1, rs2, RegWrite);
		uint32_t immediate = immgen.getImmediate(cur_instruction);

		// Set control lines of CSR
		csr.set_control_lines(CSR_en, trap_taken, mcause, pc_addr);

		// Execute instruction
		// Get first ALU operand
		uint32_t A = (PC_select == 1) ? pc_addr : rf.read_rs1();
		// Get second ALU operand
		uint32_t B;
		if (ALUSrc == 0)
			B = 4; // +4
		else if (ALUSrc == 1)
			B = rf.read_rs2(); // Register Source 2
		else
			B = immediate; // Immediate

		// Control lines for ALU Control
		uint32_t alu_opcode = aluc.getALUoperation(ALUop, funct7, funct3);
		
		// Execute operations upon operands
		uint32_t alu_output = alu.exec(A, B, alu_opcode);

		// Read data memory if applicable
		dram.set_control_signals(MemRead, MemWrite, 4, this->IO_we);
		uint32_t data_out = dram.read_data(alu_output);

		// Get register destination of CSR is applicable
		uint32_t csr_rd = csr.get_csr(rf.read_rs1());

		// Write back stage
		// Select which data to write back
		// Write to register file if applicable
		uint32_t rf_data_writeback;
		if (MemToReg == 0)
			rf_data_writeback = alu_output; // write ALU output back to register file
		else if (MemToReg == 1)
			rf_data_writeback = data_out; // write Data Memory to register
		else
			rf_data_writeback = csr_rd; // write csr register to register file
		
		// Write data to register file if applicable
		rf.write(rf_data_writeback, rd);

		// Write to memory if applicable
		dram.write_data(alu_output, rf.read_rs2());

		// Write to CSR if applicable
		uint32_t csr_addr = (cur_instruction >> 20) & 0xfff;
		csr.update_csr(csr_addr, rf.read_rs1());

		// Calculate next PC
		nextpc.calculateNextPC(immediate, opcode, funct3, A, B, csr.get_csr(0x003), trap_taken);

		// Set next PC to current PC
		pc.setPC(nextpc.getNextPC());
		this->interrupt_assert = 0; // signal back to low after one cycle

		// Erase previous frame
		//Clear(olc::BLACK);
		// if (this->interrupt_assert) {
		// 	Clear(olc::BLACK);
		// 	for(int x = 0; x < ScreenWidth(); x++) {
		// 		int temp = x / 10;
		// 		if (temp % 2 == 0) continue;
		// 		for (int y = 0; y < ScreenHeight(); y++) {
		// 			Draw(x, y, olc::Pixel(rand() % 256, rand() % 256, rand() % 256));
		// 		}
		// 	}
		// 	this->interrupt_assert = 0;
		// }
		return true;
	}
};

int main(int argc, char** argv)
{
	if (argc == 2) {
		// Check second argument
	}

	uint32_t instructions[NUM_INSTRUCTIONS] = {
		0x00f58593,		// addi a1, a1, 0xf
		0x00f58593,		// addi a1, a1, 0xf
		0x01e5b593,		// sltiu a1, 0x1e
		0xfff5c593		// xori a1, a1, 0xfff
	};
	uint32_t cur_instr_addr = 0x0;
	uint32_t instr_addr_size = 0x4;
	dram.set_control_signals(1, 1, 4, 1);
	// Insert instructions into memory
	for (uint32_t x : instructions) {
		dram.write_data(cur_instr_addr, x);
		cur_instr_addr += instr_addr_size;
	}

	// Start simulation
	CPU t89;
	if (t89.Construct(200, 200, 2, 2))
		t89.Start();
	
	exit(EXIT_SUCCESS);
}
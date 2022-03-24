// #define OLC_PGE_APPLICATION
#ifndef CPU_H
#define CPU_H

#include "olcPixelGameEngine.h"
#include "Components.h"

class CPU : public olc::PixelGameEngine
{
private:
	uint32_t IO_BUS;
	uint32_t IO_we;
	uint32_t IO_addr;
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

	void debug_pre_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void debug_post_execute(uint32_t, uint32_t, uint32_t);
	void query_external_interrupt();

public:
	CPU(std::multimap<uint32_t, uint32_t>, int);
	bool OnUserCreate() override;
	bool OnUserUpdate(float) override;
};

#endif // CPU_H
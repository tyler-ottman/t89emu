// #define OLC_PGE_APPLICATION
#ifndef PIPELINE_H
#define PIPELINE_H

#include "olcPixelGameEngine.h"
#include "Components.h"

class Pipeline : public olc::PixelGameEngine {
private:
	int interrupt_assert;
	int debug;
	uint32_t IO_BUS;
	uint32_t IO_ADDR;

	ALU alu;
	ALUControlUnit aluc;
	CSR csr;
	ImmediateGenerator immgen;
	Memory dram;
	MemControlUnit mcu;
	NextPC nextpc;
	ProgramCounter pc;
	RegisterFile rf;
	Debug debug;

	void debug_pre_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void debug_post_execute(uint32_t, uint32_t, uint32_t);
	void query_external_interrupt();

public:
	Pipeline(std::multimap<uint32_t, uint32_t>, int);
	bool OnUserCreate() override;
	bool OnUserUpdate(float) override;
};

#endif // PIPELINE_H

#ifndef DEBUG_H
#define DEBUG_H

class Debug {
public:
	void debug_post_execute(uint32_t, uint32_t, uint32_t);
	void debug_pre_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
}

#endif // DEBUG_H
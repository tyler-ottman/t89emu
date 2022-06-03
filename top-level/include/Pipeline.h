#include <stdint.h>

#ifndef PIPELINE_H
#define PIPELINE_H

#include "Components.h"
// #define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class Pipeline : public olc::PixelGameEngine {
private:
	int interrupt_assert;
	int debug_mode;
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
	void query_external_interrupt();
	void debug_post_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void debug_pre_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);


public:
	Pipeline(std::multimap<uint32_t, uint32_t>, int);
	bool OnUserCreate() override;
	bool OnUserUpdate(float) override;
};

#endif // PIPELINE_H
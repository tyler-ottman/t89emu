#include <stdint.h>
#include <map>
#include <iostream>
#include <fstream>

#ifndef PIPELINE_H
#define PIPELINE_H

#include "Components.h"

class Pipeline {
private:
	int interrupt_assert;
	int debug_mode;
	uint32_t IO_BUS;
	uint32_t IO_ADDR;
	
	void debug_post_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void debug_pre_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

public:
	Pipeline(uint32_t, uint32_t, uint32_t, uint32_t, int);
	~Pipeline();
	bool next_instruction();

	RegisterFile* rf;
	Bus* bus;
	ProgramCounter* pc;
	CSR* csr;
	ALU* alu;
	ALUControlUnit* aluc;
	ImmediateGenerator* immgen;
	MemControlUnit* mcu;
	NextPC* nextpc;
};

#endif // PIPELINE_H
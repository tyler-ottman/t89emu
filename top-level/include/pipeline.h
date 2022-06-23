#include <stdint.h>
#include <map>
#include <iostream>
#include <fstream>

#ifndef PIPELINE_H
#define PIPELINE_H

#include "Components.h"

// #define ROM_START 0x00000000
// #define RAM_START 0x80000000

class Pipeline {
private:
	int interrupt_assert;
	int debug_mode;
	uint32_t IO_BUS;
	uint32_t IO_ADDR;

	ALU alu;
	ALUControlUnit aluc;
	CSR csr;
	ImmediateGenerator immgen;
	// Memory dram;
	MemControlUnit mcu;
	NextPC nextpc;
	ProgramCounter pc;
	
	void debug_post_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void debug_pre_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

public:
	Pipeline(char*, char*, int);
	bool next_instruction();
	RegisterFile rf;
	Memory dram;
};

#endif // PIPELINE_H
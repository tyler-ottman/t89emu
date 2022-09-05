#include <stdint.h>
#include <map>
#include <iostream>
#include <fstream>

#ifndef CPU_H
#define CPU_H

#include "ALU.h"
#include "ALUControlUnit.h"
#include "Bus.h"
#include "CSR.h"
#include "ImmediateGenerator.h"
#include "MemControlUnit.h"
#include "NextPC.h"
#include "ProgramCounter.h"
#include "RegisterFile.h"
#include "Trap.h"

class CPU {
private:
	// Execute a normal RV32I instruction
	// If the instruction is executed successfully, function returns true
	// Otherwise, function returns false and stores the type of exception
	// which will then invoke the CPU to take_trap()
	uint32_t execute_instruction();
	
	void debug_post_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void debug_pre_execute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

public:
	CPU(uint32_t, uint32_t, uint32_t, uint32_t, int);
	~CPU();
	void next_instruction();

	RegisterFile* rf;
	Bus* bus;
	Trap* trap;
	ProgramCounter* pc;
	CSR* csr;
	ALU* alu;
	ALUControlUnit* aluc;
	ImmediateGenerator* immgen;
	MemControlUnit* mcu;
	NextPC* nextpc;
};

#endif // CPU_H
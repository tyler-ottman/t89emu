#include <stdint.h>
#include <map>
#include <iostream>
#include <fstream>

#ifndef CPU_H
#define CPU_H

#include "Alu.h"
#include "AluControlUnit.h"
#include "Bus.h"
#include "Csr.h"
#include "ImmediateGenerator.h"
#include "MemControlUnit.h"
#include "NextPc.h"
#include "ProgramCounter.h"
#include "RegisterFile.h"
#include "Trap.h"

class Cpu {
private:
	// Execute a normal RV32I instruction
	// If the instruction is executed successfully, function returns true
	// Otherwise, function returns false and stores the type of exception
	// which will then invoke the CPU to take_trap()
	uint32_t executeInstruction();
	
	void debugPostExecute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	void debugPreExecute(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

public:
	Cpu(uint32_t, uint32_t, uint32_t, uint32_t, int);
	~Cpu();
	void nextInstruction();

	RegisterFile *rf;
	Bus *bus;
	Trap *trap;
	ProgramCounter *pc;
	Csr *csr;
	Alu *alu;
	AluControlUnit *aluc;
	ImmediateGenerator *immgen;
	MemControlUnit *mcu;
	NextPc *nextPc;
};

#endif // CPU_H
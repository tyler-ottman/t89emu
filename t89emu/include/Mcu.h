#include <stdint.h>
#include <map>
#include <iostream>
#include <fstream>

#ifndef MCU_H
#define MCU_H

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

class Mcu {
public:
    static Mcu *getInstance(uint32_t romBase = 0, uint32_t ramBase = 0,
                            uint32_t entryPc = 0);

    void nextInstruction(void);

    Alu *getAluModule(void);
    AluControlUnit *getAluControlUnitModule(void);
    Bus *getBusModule(void);
    Csr *getCsrModule(void);
    ImmediateGenerator *getImmediateGeneratorModule(void);
    MemControlUnit *getMemControlUnitModule(void);
    NextPc *getNextPcModule(void);
    ProgramCounter *getProgramCounterModule(void);
    RegisterFile *getRegisterFileModule(void);
    Trap *getTrapModule(void);

#ifndef BUS_EXPERIMENTAL
#else
    RomMemoryDevice *getRomDevice(void) {return rom;}
    RamMemoryDevice *getRamDevice(void) {return ram;}
    VideoMemoryDevice *getVideoDevice(void) {return vram;}
    ClintMemoryDevice *getClintDevice(void) {return clint;}
#endif // BUS_EXPERIMENTAL

private:
    Mcu(uint32_t romBase, uint32_t ramBase, uint32_t entryPc);
    ~Mcu();

    // Execute a normal RV32I instruction
    // If the instruction is executed successfully, function returns true
    // Otherwise, function returns false and stores the type of exception
    // which will then invoke the CPU to take_trap()
    uint32_t executeInstruction(void);

    void debugPreExecute(uint32_t opcode, uint32_t funct3, uint32_t funct7,
                         uint32_t rs1, uint32_t rs2, uint32_t rd,
                         uint32_t immediate, uint32_t csrAddr,
                         uint32_t curInstruction);
    void debugPostExecute(uint32_t opcode, uint32_t rd, uint32_t immediate,
                          uint32_t rdData, uint32_t rs2Data,
                          uint32_t rs1Data, uint32_t pcAddr);

    // RISC Core-specific modules 
    Alu *alu;
    AluControlUnit *aluc;
    Bus *bus;
    Csr *csr;
    ImmediateGenerator *immgen;
    MemControlUnit *mcu;
    NextPc *nextPc;
    ProgramCounter *pc;
    RegisterFile *rf;
    Trap *trap;

#ifndef BUS_EXPERIMENTAL
#else
    // Peripheral modules
    RomMemoryDevice *rom;
    RamMemoryDevice *ram;
    VideoMemoryDevice *vram;
    ClintMemoryDevice *clint;
#endif // BUS_EXPERIMENTAL

    static Mcu *instance;
};

#endif // MCU_H
#ifndef TRAP_H
#define TRAP_H

#include "ClintMemoryDevice.h"
#include "Csr.h"
#include "NextPc.h"
#include "ProgramCounter.h"

class Trap {
public:
    Trap(void);
    ~Trap();

    void takeTrap(Csr *csr, ProgramCounter *pc, NextPc *nextPc,
                  uint32_t mcause);
};

#endif // TRAP_H
#ifndef TRAP_H
#define TRAP_H

#include "CSR.h"
#include "ClintMemoryDevice.h"
#include "ProgramCounter.h"
#include "NextPC.h"

class Trap {
public:
    Trap();
    ~Trap();
    void take_trap(CSR*, ProgramCounter*, NextPC*, uint32_t);
};

#endif // TRAP_H
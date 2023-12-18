#ifndef TRAP_H
#define TRAP_H

#include "Csr.h"
#include "ClintMemoryDevice.h"
#include "ProgramCounter.h"
#include "NextPc.h"

class Trap {
public:
    Trap();
    ~Trap();
    void takeTrap(Csr *, ProgramCounter *, NextPc *, uint32_t);
};

#endif // TRAP_H
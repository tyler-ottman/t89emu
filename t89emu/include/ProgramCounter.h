#include <stdint.h>

#ifndef PROGRAMCOUNTER_H
#define PROGRAMCOUNTER_H

class ProgramCounter
{
public:
    uint32_t PC;
    
    ProgramCounter();
    void setPC(uint32_t);
    uint32_t getPC();
};

#endif // PROGRAMCOUNTER_H
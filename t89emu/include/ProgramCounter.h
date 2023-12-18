#include <stdint.h>

#ifndef PROGRAMCOUNTER_H
#define PROGRAMCOUNTER_H

class ProgramCounter {
public:
    ProgramCounter(void);
    ~ProgramCounter();
    
    void setPc(uint32_t nextPc);
    uint32_t getPc(void);
    uint32_t *getPcPtr(void);

private:
    uint32_t pc;
};

#endif // PROGRAMCOUNTER_H
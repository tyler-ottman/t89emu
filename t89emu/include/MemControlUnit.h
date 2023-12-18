#include <stdint.h>

#include "Architecture.h"

#ifndef MEMCONTROLUNIT_H
#define MEMCONTROLUNIT_H

class MemControlUnit {
public:
    MemControlUnit(void);
    ~MemControlUnit();
    
    int getMemSize(uint32_t funct3);
};

#endif // MEMCONTROLUNIT_H
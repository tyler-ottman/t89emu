#include <stdint.h>

#include "Architecture.h"

#ifndef MEMCONTROLUNIT_H
#define MEMCONTROLUNIT_H

class MemControlUnit {
public:
    int getMemSize(uint32_t);
};

#endif // MEMCONTROLUNIT_H
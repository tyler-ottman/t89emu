#include <stdint.h>
#include "Architecture.h"

#ifndef MEMCONTROLUNIT_H
#define MEMCONTROLUNIT_H

class MemControlUnit
{
public:
    int get_mem_size(uint32_t);
};

#endif // MEMCONTROLUNIT_H
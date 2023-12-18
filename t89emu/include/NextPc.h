#include <stdint.h>

#include "Architecture.h"

#ifndef NEXTPC_H
#define NEXTPC_H

class NextPc {
private:
    int branchAlu(uint32_t, uint32_t, uint32_t);
public:
    NextPc();
    uint32_t calculateNextPc(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    uint32_t nextPc;
};

#endif // NEXTPC_H
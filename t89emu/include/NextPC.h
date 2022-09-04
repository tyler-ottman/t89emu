#include <stdint.h>
#include "Architecture.h"

#ifndef NEXTPC_H
#define NEXTPC_H

class NextPC {
private:
    int branch_alu(uint32_t, uint32_t, uint32_t);
public:
    NextPC();
    uint32_t calculateNextPC(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    uint32_t nextPC;
};

#endif // NEXTPC_H
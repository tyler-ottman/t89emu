#include <stdint.h>

#include "Architecture.h"

#ifndef NEXTPC_H
#define NEXTPC_H

class NextPc {
public:
    NextPc(void);
    ~NextPc();

    uint32_t calculateNextPc(uint32_t offset, uint32_t opcode, uint32_t funct3,
                             uint32_t A, uint32_t B, uint32_t mepc);

    uint32_t getNextPc(void);
    void setNextPc(uint32_t nextPc);

   private:
    int branchAlu(uint32_t A, uint32_t B, uint32_t funct3);

    uint32_t nextPc;
};

#endif // NEXTPC_H
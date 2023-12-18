#include <stdint.h>

#include "Architecture.h"

#ifndef IMMEDIATEGENERATOR_H
#define IMMEDIATEGENERATOR_H

class ImmediateGenerator {
public:
    ImmediateGenerator(void);
    ~ImmediateGenerator();
    
    uint32_t getImmediate(uint32_t instruction);
};

#endif // IMMEDIATEGENERATOR_H
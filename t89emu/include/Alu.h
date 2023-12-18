#include <stdint.h>

#ifndef ALU_H
#define ALU_H

#define ADD  0
#define SUB  1
#define OR   2
#define AND  3
#define XOR  4
#define SRL  5
#define SRA  6
#define SLL  7
#define SLT  8
#define SLTU 9

class Alu {
public:
    uint32_t execute(uint32_t, uint32_t, int);
};

#endif // ALU_H
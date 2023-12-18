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
    Alu(void);
    ~Alu();
    
    uint32_t execute(uint32_t A, uint32_t B, int aluOp);
};

#endif // ALU_H
#include <vector>

#include "Architecture.h"

#ifndef ALUCONTROLUNIT_H
#define ALUCONTROLUNIT_H

class AluControlUnit {
public:
    AluControlUnit(void);
    ~AluControlUnit();

    int getAluOperation(int opcode, int funct7, int funct3);

private:
    std::vector<int> iOperations;
    std::vector<int> rOperations;
};

#endif // ALUCONTROLUNIT_H
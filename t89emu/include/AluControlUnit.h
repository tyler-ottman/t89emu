#include <vector>

#include "Architecture.h"

#ifndef ALUCONTROLUNIT_H
#define ALUCONTROLUNIT_H

class AluControlUnit
{
private:
    std::vector<int> iOperations;
    std::vector<int> rOperations;

public:
    AluControlUnit();
    int getAluOperation(int, int, int);
};

#endif // ALUCONTROLUNIT_H
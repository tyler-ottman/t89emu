#include <vector>
#include "Architecture.h"

#ifndef ALUCONTROLUNIT_H
#define ALUCONTROLUNIT_H

class ALUControlUnit
{
private:
    std::vector<int> i_operations;
    std::vector<int> r_operations;

public:
    ALUControlUnit();
    int getALUoperation(int, int, int);
};

#endif // ALUCONTROLUNIT_H
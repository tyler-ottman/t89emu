#include <iostream>
#include "Components.h"

ProgramCounter::ProgramCounter()
{
    PC = 0;
}

void ProgramCounter::setPC(uint32_t nextPC)
{
    this->PC = nextPC;
}

uint32_t ProgramCounter::getPC()
{
    return this->PC;
}

class ProgramCounter;
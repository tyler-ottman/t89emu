#include <iostream>
#include "Components.h"

uint32_t PC;

ProgramCounter::ProgramCounter()
{
    this->PC = 0;
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
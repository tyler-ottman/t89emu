#include <iostream>
#include <unordered_map>
#include "Components.h"

uint32_t data_out;                           // Data output
uint32_t instr_out;                          // Instrution output
std::unordered_map<uint32_t, uint32_t> dram; // 16 GB memory module
int MemRead2;
int MemWrite2;
int size;
int IO_WR_enable;

Memory::Memory() {}
void Memory::set_control_signals(int MemReadData, int MemWriteData, int size, int IO_WR_enable)
{
    this->MemReadData = MemReadData;
    this->MemWriteData = MemWriteData;
    this->size = size;
    this->IO_WR_enable = IO_WR_enable;
}

void Memory::write_data(uint32_t addr, uint32_t data)
{
    dram[addr] = data;
}

void Memory::write_io(uint32_t addr, uint32_t data) {
    if(this->IO_WR_enable) {
        dram[addr] = data;
    }
}

// Hardware has separate bus lines for reading data, instructions
uint32_t Memory::read_data(uint32_t addr)
{
    if (dram.find(addr) == dram.end())
    {
        // Nothing written to address yet
        return 0;
    }
    // Something found at address
    return dram[addr];
}

class Memory;
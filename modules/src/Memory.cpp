#include <iostream>
#include <unordered_map>
#include "Components.h"

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
    if (this->MemWriteData) {
        dram[addr] = data;
        if ((addr >= VRAM_START) && (addr <= (VRAM_START + VRAM_LEN))) {
            // VGA module to update pixel in next cycle
            this->changed_pixel = addr;
        }
    }
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

uint32_t Memory::get_changed_pixel() {
    uint32_t rtn = this->changed_pixel;
    this->changed_pixel = 0x00000000; // reset changed pixel state
    return rtn;
}

class Memory;
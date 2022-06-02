#include <iostream>
#include <unordered_map>
#include "Components.h"

void Memory::write(uint32_t address, uint32_t data, int size)
{
    int offset = address & 0b11;
    uint32_t base_addr = address - offset;
    uint32_t old_data = dram[base_addr];
    switch(size) {
        case BYTE:
            switch (offset) {
                case 0b00: this->dram[base_addr] = (old_data & 0xffffff00) | (data); break;
                case 0b01: this->dram[base_addr] = (old_data & 0xffff00ff) | (data << 8); break;
                case 0b10: this->dram[base_addr] = (old_data & 0xff00ffff) | (data << 16); break;
                case 0b11: this->dram[base_addr] = (old_data & 0x00ffffff) | (data << 24); break;
            }
            break;
        case HALFWORD:
            switch (offset) {
                case 0b00: this->dram[base_addr] = (old_data & 0xffff0000) | (data); break;
                case 0b10: this->dram[base_addr] = (old_data & 0x0000ffff) | (data << 16); break;
            }
            break;
        case WORD:
            this->dram[base_addr] = data;
            break;
    }

    if ((address >= VRAM_START) && (address <= (VRAM_START + VRAM_LEN))) {
        // VGA module to update pixel in next cycle
        this->changed_pixel = address;
    }
}

// Hardware has separate bus lines for reading data, instructions
uint32_t Memory::read(uint32_t address, int size)
{
    int offset = address & 0b11;
    uint32_t base_addr = address - offset;
    uint32_t data = this->dram[base_addr];

    switch(size) {
        case BYTE:
            switch(offset) {
                case 0b00: return (data & 0x000000ff);
                case 0b01: return (data & 0x0000ff00) >> 8;
                case 0b10: return (data & 0x00ff0000) >> 16;
                case 0b11: return (data & 0xff000000) >> 24;
            }
        case HALFWORD:
            switch(offset) {
                case 0b00: return (data & 0x0000ffff);
                case 0b10: return (data & 0xffff0000) >> 16;
            }
        default: // WORD
            return data;
    }
}

// Remove when new GUI supported
uint32_t Memory::get_changed_pixel() {
    uint32_t rtn = this->changed_pixel;
    this->changed_pixel = 0x00000000; // reset changed pixel state
    return rtn;
}

class Memory;
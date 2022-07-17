#include <iostream>
#include <unordered_map>
#include "Components.h"

void Memory::write(uint32_t address, uint32_t data, int size)
{
    int offset = address & 0b11;
    uint32_t* mem_section;
    uint32_t base_addr;
    // Which memory section to be accessed
    if (address < DATA_MEMORY_START) {
        mem_section = this->instruction_memory; // Instruction Memory
        base_addr = address - offset - INSTRUCTION_MEMORY_START;
    } else if (address < VIDEO_MEMORY_START) {
        mem_section = this->data_memory; // Data Memory
        base_addr = address - offset - DATA_MEMORY_START;
    } else {
        mem_section = this->video_memory; // Video Memory
        base_addr = address - offset - VIDEO_MEMORY_START;
    }
    base_addr /= WORD;
    uint32_t old_data = mem_section[base_addr];

    switch(size) {
        case BYTE:
            switch (offset) {
                case 0b00: mem_section[base_addr] = (old_data & 0x00ffffff) | (data << 24); break;
                case 0b01: mem_section[base_addr] = (old_data & 0xff00ffff) | (data << 16); break;
                case 0b10: mem_section[base_addr] = (old_data & 0xffff00ff) | (data << 8); break;
                case 0b11: mem_section[base_addr] = (old_data & 0xffffff00) | (data); break;
            }
            break;
        case HALFWORD:
            switch (offset) {
                case 0b00: mem_section[base_addr] = (old_data & 0x0000ffff) | (data << 16); break;
                case 0b10: mem_section[base_addr] = (old_data & 0xffff0000) | (data); break;
            }
            break;
        case WORD:
            mem_section[base_addr] = data;
            break;
    }
}

// Hardware has separate bus lines for reading data, instructions
uint32_t Memory::read(uint32_t address, int size)
{
    int offset = address & 0b11;
    uint32_t* mem_section;
    uint32_t base_addr;

    // Which memory section to be accessed
    if (address < DATA_MEMORY_START) {
        mem_section = this->instruction_memory; // Instruction Memory
        base_addr = address - offset - INSTRUCTION_MEMORY_START;
    } else if (address < VIDEO_MEMORY_START) {
        mem_section = this->data_memory; // Data Memory
        base_addr = address - offset - DATA_MEMORY_START;
    } else {
        mem_section = this->video_memory; // Video Memory
        base_addr = address - offset - VIDEO_MEMORY_START;
    }
    
    base_addr /= 4;
    uint32_t data = mem_section[base_addr];

    switch(size) {
        case BYTE:
            switch(offset) {
                case 0b00: return (data & 0xff000000) >> 24;
                case 0b01: return (data & 0x00ff0000) >> 16;
                case 0b10: return (data & 0x0000ff00) >> 8;
                case 0b11: return (data & 0x000000ff);
            }
        case HALFWORD:
            switch(offset) {
                case 0b00: return (data & 0xffff0000) >> 16;
                case 0b10: return (data & 0x0000ffff);
            }
        case WORD: // WORD
            return data;
    }
    return data;
}

class Memory;
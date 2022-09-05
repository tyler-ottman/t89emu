#include "Bus.h"

// RAM / ROM defined by user in linker script
Bus::Bus(uint32_t rom_start, uint32_t rom_size, uint32_t ram_start, uint32_t ram_size) {
    rom_base = rom_start;
    rom_end = rom_start + rom_size;
    ram_base = ram_start;
    ram_end = ram_start + ram_size;

    rom_device = new ROMMemoryDevice(rom_start, rom_size);
    ram_device = new RAMMemoryDevice(ram_start, ram_size);
    video_device = new VideoMemoryDevice(VIDEO_BASE, VIDEO_SIZE);
    clint_device = new ClintMemoryDevice(CLINT_BASE, CLINT_SIZE);
}

Bus::~Bus() {
    delete rom_device;
    delete ram_device;
    delete video_device;
    delete clint_device;
}

uint32_t Bus::write(uint32_t addr, uint32_t data, uint32_t access_size) {
    // Check if addresse falls within a valid range in memory
    if ((addr >= rom_base) && (addr < rom_end)) return(rom_device->write(addr, data, access_size));
    else if ((addr >= ram_base) && (addr < ram_end)) return(ram_device->write(addr, data, access_size));
    else if ((addr >= VIDEO_BASE) && (addr < VIDEO_END)) return(video_device->write(addr, data, access_size));
    else if ((addr >= CLINT_BASE) && (addr < CLINT_END)) return(clint_device->write(addr, data, access_size));
    else {
        // Invalid address access
        return STORE_ACCESS_FAULT;
    }
}

uint32_t Bus::read(uint32_t addr, uint32_t access_size, uint32_t* read_value) {
    if ((addr >= rom_base) && (addr < rom_end)) return(rom_device->read(addr, access_size, read_value));
    else if ((addr >= ram_base) && (addr < ram_end)) return(ram_device->read(addr, access_size, read_value));
    else if ((addr >= VIDEO_BASE) && (addr < VIDEO_END)) return(video_device->read(addr, access_size, read_value));
    else if ((addr >= CLINT_BASE) && (addr < CLINT_END)) return(clint_device->read(addr, access_size, read_value));
    else {
        // Invalid address access
        return LOAD_ACCESS_FAULT;
    }
}
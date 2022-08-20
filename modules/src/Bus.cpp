#include "Components.h"

// RAM / ROM defined by user in linker script
Bus::Bus(uint32_t rom_start, uint32_t rom_size, uint32_t ram_start, uint32_t ram_size) {
    rom_base = rom_start;
    rom_end = rom_start + rom_size;
    ram_base = ram_start;
    ram_end = ram_start + ram_size;

    rom_device = new ROMMemoryDevice(rom_start, rom_size);
    ram_device = new RAMMemoryDevice(ram_start, ram_size);
    video_device = new VideoMemoryDevice(VIDEO_BASE, VIDEO_SIZE);
    csr_device = new CSRMemoryDevice(CSR_BASE, CSR_SIZE);
}

Bus::~Bus() {
    delete rom_device;
    delete ram_device;
    delete video_device;
    delete csr_device;
}

void Bus::write(uint32_t addr, uint32_t data, uint32_t access_size) {
    if ((addr >= rom_base) && (addr < rom_end)) rom_device->write(addr, data, access_size);
    else if ((addr >= ram_base) && (addr < ram_end)) ram_device->write(addr, data, access_size);
    else if ((addr >= VIDEO_BASE) && (addr < VIDEO_END)) video_device->write(addr, data, access_size);
    else if ((addr >= CSR_BASE) && (addr < CSR_END)) csr_device->write(addr, data, access_size);
    else {
        // Invalid address
    }
}

uint32_t Bus::read(uint32_t addr, uint32_t access_size) {
    if ((addr >= rom_base) && (addr < rom_end)) return(rom_device->read(addr, access_size));
    else if ((addr >= ram_base) && (addr < ram_end)) return(ram_device->read(addr, access_size));
    else if ((addr >= VIDEO_BASE) && (addr < VIDEO_END)) return(video_device->read(addr, access_size));
    else if ((addr >= CSR_BASE) && (addr < CSR_END)) return(csr_device->read(addr, access_size));
    else {
        return 0;
        // Invalid address exception (trap handler later)
    }
}
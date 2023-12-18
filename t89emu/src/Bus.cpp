#include "Bus.h"

// RAM / ROM defined by user in linker script
Bus::Bus(uint32_t romStart, uint32_t romSize, uint32_t ramStart, uint32_t ramSize) {
    romBase = romStart;
    romEnd = romStart + romSize;
    ramBase = ramStart;
    ramEnd = ramStart + ramSize;

    romDevice = new RomMemoryDevice(romStart, romSize);
    ramDevice = new RamMemoryDevice(ramStart, ramSize);
    videoDevice = new VideoMemoryDevice(VIDEO_BASE, VIDEO_SIZE);
    clintDevice = new ClintMemoryDevice(CLINT_BASE, CLINT_SIZE);
}

Bus::~Bus() {
    delete romDevice;
    delete ramDevice;
    delete videoDevice;
    delete clintDevice;
}

uint32_t Bus::write(uint32_t addr, uint32_t data, uint32_t accessSize) {
    // Check if addresse falls within a valid range in memory
    if ((addr >= romBase) && (addr < romEnd)) return(romDevice->write(addr, data, accessSize));
    else if ((addr >= ramBase) && (addr < ramEnd)) return(ramDevice->write(addr, data, accessSize));
    else if ((addr >= VIDEO_BASE) && (addr < VIDEO_END)) return(videoDevice->write(addr, data, accessSize));
    else if ((addr >= CLINT_BASE) && (addr < CLINT_END)) return(clintDevice->write(addr, data, accessSize));
    else {
        // Invalid address access
        return STORE_ACCESS_FAULT;
    }
}

uint32_t Bus::read(uint32_t addr, uint32_t accessSize, uint32_t *readValue) {
    if ((addr >= romBase) && (addr < romEnd)) return(romDevice->read(addr, accessSize, readValue));
    else if ((addr >= ramBase) && (addr < ramEnd)) return(ramDevice->read(addr, accessSize, readValue));
    else if ((addr >= VIDEO_BASE) && (addr < VIDEO_END)) return(videoDevice->read(addr, accessSize, readValue));
    else if ((addr >= CLINT_BASE) && (addr < CLINT_END)) return(clintDevice->read(addr, accessSize, readValue));
    else {
        // Invalid address access
        return LOAD_ACCESS_FAULT;
    }
}
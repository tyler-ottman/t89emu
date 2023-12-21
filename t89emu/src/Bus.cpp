#include "Bus.h"

#ifndef BUS_EXPERIMENTAL

// RAM / ROM defined by user in linker script
Bus::Bus(uint32_t romStart, uint32_t romSize, uint32_t ramStart,
         uint32_t ramSize) {
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

uint32_t Bus::read(uint32_t addr, uint32_t accessSize, uint32_t *readValue) {
    if ((addr >= romDevice->getBaseAddress()) &&
        (addr < romDevice->getEndAddress())) {
        return (romDevice->read(addr, accessSize, readValue));
    } else if ((addr >= ramDevice->getBaseAddress()) &&
               (addr < ramDevice->getEndAddress())) {
        return (ramDevice->read(addr, accessSize, readValue));
    } else if ((addr >= videoDevice->getBaseAddress()) &&
               (addr < videoDevice->getEndAddress())) {
        return (videoDevice->read(addr, accessSize, readValue));
    } else if ((addr >= clintDevice->getBaseAddress()) &&
               (addr < clintDevice->getEndAddress())) {
        return (clintDevice->read(addr, accessSize, readValue));
    } else {
        // Invalid address access
        return LOAD_ACCESS_FAULT;
    }
}

uint32_t Bus::write(uint32_t addr, uint32_t data, uint32_t accessSize) {
    // Check if addresse falls within a valid range in memory
    if ((addr >= romDevice->getBaseAddress()) &&
        (addr < romDevice->getEndAddress())) {
        return (romDevice->write(addr, data, accessSize));
    } else if ((addr >= ramDevice->getBaseAddress()) &&
               (addr < ramDevice->getEndAddress())) {
        return (ramDevice->write(addr, data, accessSize));
    } else if ((addr >= videoDevice->getBaseAddress()) &&
               (addr < videoDevice->getEndAddress())) {
        return (videoDevice->write(addr, data, accessSize));
    } else if ((addr >= clintDevice->getBaseAddress()) &&
               (addr < clintDevice->getEndAddress())) {
        return (clintDevice->write(addr, data, accessSize));
    } else {
        // Invalid address access
        return STORE_ACCESS_FAULT;
    }
}

uint32_t Bus::getRomBase() {
    return romBase;
}

uint32_t Bus::getRomEnd() {
    return romEnd;
}

uint32_t Bus::getRamBase() {
    return ramBase;
}

uint32_t Bus::getRamEnd() {
    return ramEnd;
}

ClintMemoryDevice *Bus::getClintDevice() {
    return clintDevice;
}

RamMemoryDevice *Bus::getRamMemoryDevice() {
    return ramDevice;
}

RomMemoryDevice *Bus::getRomMemoryDevice() {
    return romDevice;
}

VideoMemoryDevice *Bus::getVideoDevice() {
    return videoDevice;
}

#else

Bus::Bus() {
    
}

Bus::~Bus() {

}

uint32_t Bus::read(uint32_t addr, uint32_t accessSize, uint32_t *readValue) {
    for (MemoryDevice *device : devices) {
        if (addr >= device->getBaseAddress() && addr <= device->getEndAddress()) {
            return device->read(addr, accessSize, readValue);
        }
    }

    return LOAD_ACCESS_FAULT;
}

uint32_t Bus::write(uint32_t addr, uint32_t data, uint32_t accessSize) {
    for (MemoryDevice *device : devices) {
        if (addr >= device->getBaseAddress() && addr <= device->getEndAddress()) {
            return device->write(addr, data, accessSize);
        }
    }

    return STORE_ACCESS_FAULT;
}

int Bus::addDevice(MemoryDevice *device) {
    devices.push_back(device);
    return devices.size() - 1;
}

#endif // BUS_EXPERIMENTAL
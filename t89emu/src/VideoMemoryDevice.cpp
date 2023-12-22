#include "VideoMemoryDevice.h"

VideoMemoryDevice::VideoMemoryDevice(uint32_t base, uint32_t size,
                                     uint32_t gWidth, uint32_t gHeight,
                                     uint32_t tWidth, uint32_t tHeight)
    : MemoryDevice::MemoryDevice(base, size),
      gWidth(gWidth),
      gHeight(gHeight),
      tWidth(tWidth),
      tHeight(tHeight) {
    
}

uint32_t VideoMemoryDevice::read(uint32_t addr, uint32_t size,
                                 uint32_t *readValue) {
    // Check memory access is aligned
    if (!checkAlignment(addr, size)) {
        return LOAD_ADDRESS_MISALIGNED;
    }

    switch (size) {
    case BYTE: *readValue = *((uint8_t *)getAddress(addr));
    case HALFWORD: *readValue = *((uint16_t *)getAddress(addr));
    case WORD: *readValue = *((uint32_t *)getAddress(addr));
    }

    return STATUS_OK;
}

uint32_t VideoMemoryDevice::write(uint32_t addr, uint32_t writeValue,
                                  uint32_t size) {
    // Check memory access is aligned
    if (!checkAlignment(addr, size)) {
        return STORE_ADDRESS_MISALIGNED;
    }

    switch (size) {
    case BYTE: *((uint8_t *)getAddress(addr)) = writeValue; break;
    case HALFWORD: *((uint16_t *)getAddress(addr)) = writeValue; break;
    case WORD : *((uint32_t *)getAddress(addr)) = writeValue; break;
    }

    return STATUS_OK;
}

uint32_t VideoMemoryDevice::getGWidth() {
    return gWidth;
}

uint32_t VideoMemoryDevice::getGHeight() {
    return gHeight;
}

uint32_t VideoMemoryDevice::getTWidth() {
    return tWidth;
}

uint32_t VideoMemoryDevice::getTHeight() {
    return tHeight;
}
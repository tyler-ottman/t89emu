#include <stdint.h>

#include "MemoryDevice.h"

#ifndef VIDEO_MEMORYDEVICE_H
#define VIDEO_MEMORYDEVICE_H

#define VGA_TEXT_MODE                   1
#define GRAPHICS_MODE                   2

class VideoMemoryDevice : public MemoryDevice {
public:
    VideoMemoryDevice(uint32_t base, uint32_t size, uint32_t gWidth,
                      uint32_t gHeight, uint32_t tWidth, uint32_t tHeight);
    
    uint32_t read(uint32_t addr, uint32_t size, uint32_t *readValue);
    uint32_t write(uint32_t addr, uint32_t value, uint32_t size);

    uint32_t getGWidth(void);
    uint32_t getGHeight(void);
    uint32_t getTWidth(void);
    uint32_t getTHeight(void);

private:
    uint32_t gWidth; // Graphics Mode Width
    uint32_t gHeight;
    uint32_t tWidth; // Text Mode Width
    uint32_t tHeight;
};

#endif // VIDEO_MEMORYDEVICE_H
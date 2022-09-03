#include <stdint.h>
#include "MemoryDevice.h"

#ifndef VIDEO_MEMORYDEVICE_H
#define VIDEO_MEMORYDEVICE_H

#define VGA_TEXT_MODE 1
#define GRAPHICS_MODE 2

#define TEXT_MODE_VERTICAL_LINES    64
#define TEXT_MODE_HORIZONTAL_LINES  21

class VideoMemoryDevice : public MemoryDevice {
public:
    VideoMemoryDevice(uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    void write(uint32_t, uint32_t, uint32_t);
};

#endif // VIDEO_MEMORYDEVICE_H
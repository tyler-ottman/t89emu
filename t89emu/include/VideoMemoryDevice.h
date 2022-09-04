#include <stdint.h>
#include "MemoryDevice.h"

#ifndef VIDEO_MEMORYDEVICE_H
#define VIDEO_MEMORYDEVICE_H

// Video Memory Device
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 288

#define VIDEO_BASE 0x20000000
#define VIDEO_TEXT_OFFSET (VIDEO_BASE + 16) // Starting address of Video Text buffer
#define VIDEO_TEXT_BUFFER_SIZE (TEXT_MODE_HORIZONTAL_LINES * TEXT_MODE_VERTICAL_LINES)
#define VIDEO_GRAPHICS_OFFSET (VIDEO_BASE + 16 + VIDEO_TEXT_BUFFER_SIZE)
#define VIDEO_SIZE (16 + VIDEO_TEXT_BUFFER_SIZE + SCREEN_WIDTH * SCREEN_HEIGHT * WORD)
#define VIDEO_END (VIDEO_BASE + VIDEO_SIZE)

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
#include <stdint.h>
#include "Architecture.h"
#include "ROMMemoryDevice.h"
#include "RAMMemoryDevice.h"
#include "VideoMemoryDevice.h"
#include "CSRMemoryDevice.h"

#ifndef BUS_H
#define BUS_H

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 288

// ROM/RAM base address defined by linker, maximum possible size is fixed
#define ROM_SIZE 2097152 // 2 MB
#define RAM_SIZE 1048576 // 1 MB

// Some memory devices are at fixed addresses

// CSR Memory-Mapped IO Device
#define CSR_BASE 0x30000000
#define CSR_SIZE 0x14
#define CSR_END (CSR_BASE + CSR_SIZE)

// Video Memory Device
#define VIDEO_BASE 0x20000000
#define VIDEO_TEXT_OFFSET (VIDEO_BASE + 16) // Starting address of Video Text buffer
#define VIDEO_TEXT_BUFFER_SIZE (TEXT_MODE_HORIZONTAL_LINES * TEXT_MODE_VERTICAL_LINES)
#define VIDEO_GRAPHICS_OFFSET (VIDEO_BASE + 16 + VIDEO_TEXT_BUFFER_SIZE)
#define VIDEO_SIZE (16 + VIDEO_TEXT_BUFFER_SIZE + SCREEN_WIDTH * SCREEN_HEIGHT * WORD)
#define VIDEO_END (VIDEO_BASE + VIDEO_SIZE)

// Future: UART
// Future: PLIC
class Bus {
    
public:
    Bus(uint32_t, uint32_t, uint32_t, uint32_t);
    ~Bus();
    void write(uint32_t, uint32_t, uint32_t);
    uint32_t read(uint32_t, uint32_t);
    ROMMemoryDevice* rom_device;
    RAMMemoryDevice* ram_device;
    VideoMemoryDevice* video_device;
    CSRMemoryDevice* csr_device;
    uint32_t rom_base;
    uint32_t rom_end;
    uint32_t ram_base;
    uint32_t ram_end;
};

#endif // BUS_H
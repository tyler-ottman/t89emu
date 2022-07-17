#include <stdlib.h>

#define VRAM_START 0x20000000
volatile unsigned int *VIDEO_MEMORY = (unsigned int *)(VRAM_START);

#define WIDTH 512
#define HEIGHT 288

#define BLUE_SCREEN 0xffbb4700

void init_screen() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) VIDEO_MEMORY[i] = BLUE_SCREEN;
}

int main(void)
{
    init_screen();
    return 0;
}
#include <stdlib.h>

#define BUF_LEN 64
#define VRAM_START (unsigned int)0x20000000
volatile unsigned char *VIDEO_MEMORY = (unsigned char *)(VRAM_START);
volatile unsigned char *TEXT_BUFFER = (unsigned char *)(VRAM_START + 16);
volatile unsigned int *GRAPHICS_BUFF = (unsigned int *)(VRAM_START + 1360);

#define WIDTH 512
#define HEIGHT 288

#define BLUE_SCREEN 0xffbb4700

void init_blue_screen() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) GRAPHICS_BUFF[i] = BLUE_SCREEN;
}

int main(void)
{
    // Enable VGA Text Mode
    *VIDEO_MEMORY = 1;

    // Write 'Hello World!' to screen
    char screen_buf[13] = "Hello World!";
    for (int idx = 0; idx < 13; idx++) {
        TEXT_BUFFER[idx] = screen_buf[idx];
    }

    // Enable Graphics Mode
    // *VIDEO_MEMORY = 2;
    init_blue_screen();

    return 0;
}

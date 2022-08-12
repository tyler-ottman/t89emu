#include <stdlib.h>

#define VRAM_START (unsigned int)0x20000000
volatile unsigned int *VIDEO_MEMORY = (unsigned int *)(VRAM_START);

#define WIDTH 512
#define HEIGHT 288

#define BLUE_SCREEN 0xffbb4700
#define GREEN_SCREEN 0xff00ff00

void init_blue_screen() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) VIDEO_MEMORY[i] = BLUE_SCREEN;
}

void init_green_screen() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) VIDEO_MEMORY[i] = GREEN_SCREEN;
}

int test_arr[10] = { 0 };

int main(void)
{
    unsigned int i;
    char* test = "hello world\n";
    while (1) {
        for (i = 0; i < 20; i++)  init_blue_screen();
        for (i = 0; i < 20; i++) {
            init_green_screen();
        }
        break;
    }
    
    // init_blue_screen();

    return 0;
}
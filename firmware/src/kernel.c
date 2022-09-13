#include <stdlib.h>
#include "terminal.h"

// #define BUF_LEN 64
// #define VRAM_START (unsigned int)0x20000000
// volatile unsigned char *VIDEO_MEMORY = (unsigned char *)(VRAM_START);
// volatile unsigned char *TEXT_BUFFER = (unsigned char *)(VRAM_START + 16);
// volatile unsigned int *GRAPHICS_BUFF = (unsigned int *)(VRAM_START + 1360);



// #define WIDTH 512
// #define HEIGHT 288

// #define BLUE_SCREEN 0xffbb4700

// void init_blue_screen() {
//     for (int i = 0; i < WIDTH * HEIGHT; i++) GRAPHICS_BUFF[i] = BLUE_SCREEN;
// }

int kernel_main(void)
{
    init_terminal();

    // Write 'Hello World!' to screen
    enable_text_mode();
    print_str("Hello World!", 13);
    print_str("Kernel test", 12);
    print_str_line("", 0);
    print_str_line("New line", 8);

    return 0;
}

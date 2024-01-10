#include "terminal.h"

terminal_state terminal;

void init_terminal(void) {
    // Initialize state of terminal
    terminal.cursor_index = 0;
}

void init_blue_screen(void) {
    volatile unsigned int* buffer = GRAPHICS_BUFF;
    int idx;
    for (idx = 0; idx < PIXEL_MAX; idx++) {
        buffer[idx] = BLUE_SCREEN;
    }
}

void print_str(const char* str, int len) {
    volatile unsigned char* buffer = TEXT_BUFFER;
    int idx;
    for (idx = 0; idx < len; idx++) {
        buffer[terminal.cursor_index++] = str[idx];
    }
}

void print_str_line(const char* str, int len) {
    volatile unsigned char* buffer = TEXT_BUFFER;
    int idx;
    for (idx = 0; idx < len; idx++) {
        buffer[terminal.cursor_index++] = str[idx];
    }
    terminal.cursor_index += 64;
}

void enable_text_mode(void) {
    // Set first byte of video memory to text buffer
    *VIDEO_MEMORY = 1;
}

void enable_graphics_mode(void) {
    // Set firty byte of video memory to graphics buffer
    *VIDEO_MEMORY = 2;
}

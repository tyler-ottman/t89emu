#ifndef TERMINAL_H
#define TERMINAL_H

// Screen Resolution 512x288 pixels
#define WIDTH 512
#define HEIGHT 288
#define PIXEL_MAX (WIDTH * HEIGHT)

// Maximum # of characters that can print to terminal
#define BUF_LEN 64

// Standard VGA Colors
#define BLUE_SCREEN 0xffbb4700

// Pointer to sections inside video controller in memory
#define VRAM_START (unsigned int)0x20000000
#define VIDEO_MEMORY (volatile unsigned char*)(VRAM_START)
#define TEXT_BUFFER (volatile unsigned char*)(VRAM_START + 16)
#define GRAPHICS_BUFF (volatile unsigned int*)(VRAM_START + 1360)

typedef struct {
    int cursor_index;
    
} terminal_state;

void init_terminal(void);
void enable_text_mode(void);
void enable_graphics_mode(void);
void init_blue_screen(void);
void print_str(const char* str, int len);
void print_str_line(const char* str, int len);


// volatile unsigned char *VIDEO_MEMORY = (unsigned char *)(VRAM_START);
// volatile unsigned char *TEXT_BUFFER = (unsigned char *)(VRAM_START + 16);
// volatile unsigned int *GRAPHICS_BUFF = (unsigned int *)(VRAM_START + 1360);

#endif // TERMINAL_H
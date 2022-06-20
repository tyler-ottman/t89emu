#define VRAM_START 0x40000000
volatile unsigned int *VIDEO_MEMORY = (unsigned int*)(VRAM_START);

#define WIDTH (int)512
#define HEIGHT (int)288

#define RED 0xff0000ff
#define GREEN 0xff00ff00
#define BLUE 0xffff0000

void colorScreen(volatile unsigned int* video_mem, unsigned int color) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        if (color == RED) {
            video_mem[i] = RED;
            color = GREEN;
        } else if (color == GREEN) {
            video_mem[i] = GREEN;
            color = BLUE;
        } else {
            video_mem[i] = BLUE;
            color = RED;
        }
    }
}

void colorGradient(volatile unsigned int* video_mem) {
    int color = 0;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        video_mem[i] = color | 0xff000000;
        color += 256;
    }
    color = 0;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        video_mem[i] = color | 0xff000000;
        color += 65536;
    }
    color = 0;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        video_mem[i] = color | 0xff000000;
        color += 1;
    }
}

// Code set up for WIDTHxHEIGHT pixel monitor with VGA reading from 0x40000000
int main(void) {
    while(1) {
        // colorScreen(VIDEO_MEMORY, 0xffff0000);
        // colorScreen(VIDEO_MEMORY, 0xff000000);
        colorGradient(VIDEO_MEMORY);
    }
    return 0;
}
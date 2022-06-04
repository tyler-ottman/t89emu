#define VRAM_START 0x40000000
volatile unsigned int *VIDEO_MEMORY = (unsigned int*)(VRAM_START);

#define WIDTH (int)50
#define HEIGHT (int)50

#define RED 0xff000000
#define GREEN 0x00ff0000
#define BLUE 0x0000ff00

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
        video_mem[i] = color;
        color += 256;
    }
}

// Code set up for WIDTHxHEIGHT pixel monitor with VGA reading from 0x40000000
int main(void) {
    colorScreen(VIDEO_MEMORY, 0x00ff0000);
    // colorScreen(VIDEO_MEMORY, 0xff000000);
    colorGradient(VIDEO_MEMORY);
    while(1){}
    return 0;
}
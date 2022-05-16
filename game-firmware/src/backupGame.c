#define VRAM_START 0x40000000
volatile unsigned int *VIDEO_MEMORY = (unsigned int*)(VRAM_START);

#define WIDTH 352
#define HEIGHT 198



void colorScreen(volatile unsigned int* video_mem, unsigned int color) {
    for (int i = 0; i < WIDTH * HEIGHT; i++)
        video_mem[i] = color;
}

void drawImage() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        VIDEO_MEMORY[i] = image[i];
    }
}

// Code set up for WIDTHxHEIGHT pixel monitor with VGA reading from 0x40000000
int main(void) {
    // colorScreen(VIDEO_MEMORY, 0x00ff0000);
    drawImage();
    while(1){}
    return 0;
}
#define VRAM_START 0x40000000
volatile unsigned int *VIDEO_MEMORY = (unsigned int*)(VRAM_START);

// Code set up for 200x200 pixel monitor with VGA reading from 0x40000000
int main(void) {
    for (int i = 0; i < 100*100; i++) {
        VIDEO_MEMORY[i] = 0x00ff0000;
    }

    while(1) {}
    return 0;
}
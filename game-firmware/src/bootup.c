typedef unsigned int uint32_t;

extern uint32_t _program_end;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void load_ram(void) {
    uint32_t* rom_data = &_program_end;     // Points to data to load into RAM from ROM
    uint32_t* ram_ptr = &_sdata;            // Initially point to first RAM address

    while (ram_ptr < &_edata) {
        *ram_ptr++ = *rom_data++;
    }

    rom_data = &_sbss;
    while (rom_data < &_ebss) {              // Unitialized data
        *rom_data++ = 0;
    }
}

// code from https://sourceware.org/newlib/libc.html#Syscalls
char *_sbrk(uint32_t incr)
{
    extern char _heapbase; /* Defined by the linker */
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0)
    {
        heap_end = &_heapbase;
    }
    prev_heap_end = heap_end;

    heap_end += incr;
    return (char *)prev_heap_end;
}

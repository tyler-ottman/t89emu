typedef unsigned int uint32_t;

extern uint32_t _program_end;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

#define MSTATUS_MIE_MASK 3

#define MIE_MEIE_MASK 11
#define MIE_MTIE_MASK 7
#define MIE_MSIE_MASK 3

// https://five-embeddev.com/code/2020/11/18/csr-access/
static inline __attribute__((always_inline)) void csr_set_field_mstatus(uint32_t mask) {
    __asm__ volatile ("csrrs x0, mstatus, %0" : "=r"(mask));
}

static inline __attribute__((always_inline)) void csr_reset_field_mstatus(uint32_t mask) {
    __asm__ volatile ("csrrc x0, mstatus, %0" : "=r"(mask));
}

static inline __attribute__((always_inline)) void csr_set_field_mie(uint32_t mask) {
    __asm__ volatile ("csrrs x0, mie, %0" : "=r"(mask));
}

static inline __attribute__((always_inline)) void csr_reset_field_mie(uint32_t mask) {
    __asm__ volatile ("csrrc x0, mie, %0" : "=r"(mask));
}

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

    // Enable Global Interrupts in mstatus
    csr_set_field_mstatus(1 << MSTATUS_MIE_MASK);

    // Enable External/Timer/Software Interrupts
    csr_set_field_mie((1 << MIE_MEIE_MASK) | (1 << MIE_MTIE_MASK) | (1 << MIE_MSIE_MASK));
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

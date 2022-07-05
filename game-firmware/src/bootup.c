typedef unsigned int uint32_t;

extern uint32_t _program_end;
extern uint32_t _sdata;
extern uint32_t _edata;

void load_ram(void) {
    uint32_t* rom_data = &_program_end;     // Points to data to load into RAM from ROM
    uint32_t* ram_ptr = &_sdata;            // Initially point to first RAM address

    while (ram_ptr < &_edata) {
        *ram_ptr++ = *rom_data++;
    }
}

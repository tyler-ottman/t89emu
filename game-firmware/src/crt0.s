.section .init
.global _start
_start:
    lui sp, 0x8001f
    addi a5, a5, 1024
    add sp, sp, -4 # sp = top of RAM = 0x80ffffff

    call load_ram

    call main

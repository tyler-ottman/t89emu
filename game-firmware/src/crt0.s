.section .init
.global _start
_start:
    lui sp, 0x81000
    add sp, sp, -4 # sp = top of RAM = 0x80ffffff 

    call main

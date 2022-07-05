OUTPUT_ARCH("riscv")

ENTRY(_start)
MEMORY
{
    ROM (rx) : ORIGIN = 0x00000000, LENGTH = 128K
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 1M
}

SECTIONS
{
    PROVIDE( __stack_top = ORIGIN(RAM) + LENGTH(RAM));

    /* Boot code, user code, read only data in ROM */
    .text :
    {
        . = ALIGN(4);
        _stext = .; /* Start of .text */
        *(.init*)   /* .init section */
        *(.vectortable*)
        *(.text*)   /* user code */
        _etext = .; /* End of .text */
        
        _srodata = .; /* Start of .rodata */
        *(.srodata*)
        *(.rodata*)
        . = ALIGN(4);
        _erodata = .; /* End of .rodata */

        _program_end = .;
    } > ROM

    .data :
    {
        PROVIDE( __global_pointer$ = . + (4K / 2) );
        _sdata = .;
        *(.sdata*)
        *(.sdata2*)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    .bss (NOLOAD):
    {
        . = ALIGN(4);
        _sbss = .; /* Start of .bss */
        *(.bss*)
        *(.sbss*)
        . = ALIGN(4);
        _ebss = .; /* End of .bss */
    } > RAM
}

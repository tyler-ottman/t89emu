.section .init
.global _start
_start:
    la sp, __stack_top                  # Load stack pointer
    addi sp, sp, -4
    .option push
    .option norelax
	la gp, __global_pointer$            # Load global pointer
    .option pop

    la a0, _vector_table  # Load Address of ISR
    csrw mtvec, a0

    # Exception testers
    # li a1, 0x20000001
    # sw a1, 0(a1)
    # ecall

    call load_ram                       # Load data to RAM
    call main                           # OS main routine

_spin:
    jal _spin

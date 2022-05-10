.global _start
_start:
    addi a2, x0, 0x1
    addi a3, x0, 0x3
loop:
    # jal end_loop
    addi a2, a2, 0x1
    nop
    nop
    jal x0, loop
end_loop:

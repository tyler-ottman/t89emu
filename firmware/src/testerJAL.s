.global _start
_start:
    addi a2, x0, 0x1
    addi a3, x0, 0x3
loop:
    # jal end_loop
    jalr x0, 16(x0)
    addi a2, a2, 0x1
    nop
    nop
    jalr x0, -8(x0)
end_loop:

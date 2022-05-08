.global _start
_start:
    addi a1, a1, 0xf
    addi a1, a1, 0xf
    addi a2, a1, 0xff
    addi a2, a2, -1
    slti a2, a1, -1
    slti a1, a1, 0xf
    sltiu a2, a1, -1
    sltiu a2, a1, 0
    xori a2, a2, -1348
    xori a2, a2, -1348
    ori a2, a1, -1366
    ori a2, a2, -1
    andi a2, a2, 0x0aa
    andi a2, a2, 0x055
    andi a2, a2, 0x0
    andi a2, a2, -1
    slli a2, a1, 0x003
    addi a2, a1, 0x002
    slli a2, a2, 0x003
    srli a2, a2, 0x001
    lui a2, 0x80000
    srai a2, a2, 0x001
    lui a2, 0x40000
    srai a2, a2, 0x001
    lui a2, 0x80000
    slti a2, a2, 0x001
    lui a2, 0xfffff
    slti a2, a2, 0x001
    lui a2, 0x80000
    sltiu a2, a2, 0x001
    addi a1, x0, 0x001
    sltiu a2, a1, 0x002
    addi x0, x0, 0x001
    addi a2, x0, 0x00e
    add a2, a2, a1
    lui a2, 0x80000
    add a2, a2, a1
    addi a2, x0, 0x008
    sub a2, a2, a1
    sub a2, a1, a2
    addi a2, x0, 0x00b
    sll a2, a2, a1
    sll a2, a2, a1
    slt a2, a1, a2
    slt a2, a2, x0
    lui a2, 0x80000
    slt a2, a2, a1
    lui a2, 0x80000
    sltu a2, a2, a1
    lui a2, 0xaaaaa
    lui a1, 0x55555
    xor a2, a2, a1
    xor a2, a2, a2
    lui a2, 0x00001
    addi a1, x0, 0x001
    srl a2, a2, a1
    lui a2, 0x80000
    sra a2, a2, a1
    addi a1, x0, -1
    or a3, a2, a1
    and a2, a2, a1
    lui a2, 0xffff0
    addi a5, x0, 0xff
    sw a5, 0(a2)
    addi a5, x0, 0x10
    sw a5, 4(a2)
    lw a3, 0(a2)
    lw a3, 4(a2)
    addi a2, x0, -32
    lui a3, 0xfffef
    or a2, a2, a3
    sw a1, 32(a2)
    lui a2, 0xffff0
    lw a3, 0(a2)

    jal instructionsBNE2

instructionsBNE2:
    addi a2, x0, 0x1
    addi a3, x0, 0x3
loop:
    bne a2, a3, end_loop
    addi a2, a2, 0x1
    jal x0, loop
end_loop:
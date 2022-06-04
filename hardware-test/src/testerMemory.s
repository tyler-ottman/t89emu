.global _start
_start:
	lui a0, 0x70000
	addi a1, zero, 0xdd
	sw a1, 0(a0)
	lw a2, 0(a0)

	lui a0, 0x6fff8
	addi a1, zero, 0xaa
	sw a1, 8(a0)
	lw a2, 8(a0)

	lui a0, 0x70000
	lui a1, 0x00001
	addi a1, a1, 0x122
	
	lui a2, 0x00003
	addi a2, a2, 0x344

	sh a1, 0(a0)
	sh a2, 2(a0)

	lh a3, 0(a0)
	lh a4, 2(a0)

	lw a5, 0(a0)
	
	lui a0, 0x60000
	addi a1, x0, 0x12
	addi a2, x0, 0x34
	addi a3, x0, 0x56
	addi a4, x0, 0x78

	sb a1, 0(a0)
	sb a2, 1(a0)
	sb a3, 2(a0)
	sb a4, 3(a0)

	lw a5, 0(a0)

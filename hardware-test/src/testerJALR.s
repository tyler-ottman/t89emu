.global _start
_start:
	nop
	nop
	nop
	nop
	nop
	jal my_func
	addi a0, x0, 0xf
	addi a1, x0, 0xff
my_func:
	lui s0, 0x0f0f0
	lui s1, 0xf0f0f
	ret

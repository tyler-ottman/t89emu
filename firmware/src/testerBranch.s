.global _start
_start:
	addi a0, x0, 5
loop:
	beq x0, a0, end_loop
	addi a0, a0, -1
	jal loop
end_loop:
	nop
	nop
	beq x0, a0, loop

.global _start
_start:
loop:
	# BNE test
	addi a2, x0, 0x1
 	addi a3, x0, 0x3
	bne a2, a3, end_loop
	addi a2, a2, 0x1
	jal x0, loop
end_loop:
	addi a3, x0, 0x2
	addi a2, x0, 0x0
	jal x0, loop2
	addi a3, x0, 0x3
loop2:
	addi a2, a2, 0x1
	bne a2, a3, loop2

	# BLT Test
	addi a3, x0, 0x3
	addi a2, x0, 0x0
	jal x0, loop3
	addi a3, x0, 0x3
loop3:
	addi a2, a2, 0x1
	blt a2, a3, loop3

	# BGE Test
	addi a3, x0, -2
	addi a2, x0, 0x3
loop4:
	addi a3, a3	, 0x1
	bge a2, a3, loop4

	# BGEU Test
	addi a2, x0, -3
	addi a3, x0, 0x1
loop5:
	addi a2, a2, 0x1
	bgeu a2, a3, loop5 

	# BLTU Test
	addi a3, x0, 0x1
	addi a2, x0, -2
loop6:
	addi a2, a2, 0x1
	bltu a3, a2, loop6

	addi a5, x0, 0xf
	addi a6, x0, 0xf
loop7:
	beq a5, a6, loop7

#include "CPU.h"
#include <map>

std::multimap<uint32_t, uint32_t> instructionsRIS = {
	{0x00000000, 0x00f58593}, // addi a1, a1, 0xf
	{0x00000004, 0x00f58593}, // addi a1, a1, 0xf
	{0x00000008, 0x0ff58613}, // addi a2, a1, 0xff
	{0x0000000c, 0xfff60613}, // addi a2, a2, 0xfff
	{0x00000010, 0xfff62613}, // slti a2, a1, 0xfff
	{0x00000014, 0x0ff5a593}, // slti a1, a1, 0xf (test signed later)
	{0x00000018, 0xfff5b613}, // sltiu a2, a1, 0xfff
	{0x0000001c, 0x0005b613}, // sltiu a2, a1, 0x0 (test "signed" later)
	{0x00000020, 0xabc64613}, // xori a2, a2, 0xabc
	{0x00000024, 0xabc64613}, // xori a2, a2, 0xabc
	{0x00000028, 0xaaa5e613}, // ori a2, a1, 0xaaa
	{0x0000002c, 0xfff66613}, // ori a2, a2, 0xfff
	{0x00000030, 0x0aa67613}, // andi a2, a2, 0x0aa
	{0x00000034, 0x05567613}, // andi a2, a2, 0x055
	{0x00000038, 0x00067613}, // andi a2, a2, 0x000
	{0x0000003c, 0xfff67613}, // andi a2, a2, 0xfff
	{0x00000040, 0x00359613}, // slli a2, a1, 0x003
	{0x00000044, 0x00258613}, // addi a2, a1, 0x002
	{0x00000048, 0x00361613}, // slli a2, a2, 0x003
	{0x0000004c, 0x00165613}, // srli a2, a2, 0x001
	{0x00000050, 0x80000637}, // lui a2, 0x80000
	{0x00000054, 0x40165613}, // srai a2, a2, 0x001
	{0x00000058, 0x40000637}, // lui a2, 0x40000
	{0x0000005c, 0x40165613}, // srai a2, a2, 0x001
	{0x00000060, 0x80000637}, // lui a2, 0x80000
	{0x00000064, 0x00162613}, // slti a2, a2, 0x001
	{0x00000068, 0xfffff637}, // lui a2, 0xfffff
	{0x0000006c, 0x00162613}, // slti a2, a2, 0x001
	{0x00000070, 0x80000637}, // lui a2, 0x80000
	{0x00000074, 0x00163613}, // sltiu a2, a2, 0x001
	{0x00000078, 0x00100593}, // addi a1, x0, 0x001
	{0x0000007c, 0x0025b613}, // sltiu a2, a1, 0x002
	{0x00000080, 0x00100013}, // addi x0, x0, 0x001
	{0x00000084, 0x00e00613}, // addi a2, x0, 0x00e
	{0x00000088, 0x00b60633}, // add a2, a2, a1
	{0x0000008c, 0x80000637}, // lui a2, 0x80000
	{0x00000090, 0x00b60633}, // add a2, a2, a1
	{0x00000094, 0x00800613}, // addi a2, x0, 0x008
	{0x00000098, 0x40b60633}, // sub a2, a2, a1
	{0x0000009c, 0x40c58633}, // sub a2, a1, a2
	{0x000000a0, 0x00b00613}, // addi a2, x0, 0x00b
	{0x000000a4, 0x00b61633}, // sll a2, a2, a1
	{0x000000a8, 0x00b61633}, // sll a2, a2, a1
	{0x000000ac, 0x00c5a633}, // slt a2, a1, a2
	{0x000000b0, 0x00062633}, // slt a2, a2, x0
	{0x000000b4, 0x80000637}, // lui a2, 0x80000
	{0x000000b8, 0x00b62633}, // slt a2, a2, a1
	{0x000000bc, 0x80000637}, // lui a2, 0x80000
	{0x000000c0, 0x00b63633}, // sltu a2, a2, a1
	{0x000000c4, 0xaaaaa637}, // lui a2, 0xaaaaa
	{0x000000c8, 0x555555b7}, // lui a1, 0x55555
	{0x000000cc, 0x00b64633}, // xor a2, a2, a1
	{0x000000d0, 0x00c64633}, // xor a2, a2, a2
	{0x000000d4, 0x00001637}, // lui a2, 0x00001
	{0x000000d8, 0x00100593}, // addi a1, x0, 0x001
	{0x000000dc, 0x00b65633}, // srl a2, a2, a1
	{0x000000e0, 0x80000637}, // lui a2, 0x00001
	{0x000000e4, 0x40b65633}, // sra a2, a2, a1
	{0x000000e8, 0xfff00593}, // addi a1, x0, 0xfff
	{0x000000ec, 0x00b666b3}, // or a3, a2, a1
	{0x000000f0, 0x00b67633}, // and a2, a2, a1
	{0x000000f4, 0xffff0637}, // lui a2, 0xffff0
	{0x000000f8, 0x00062683}, // lw a3, 0(a2)
	{0x000000fc, 0x00462683}, // lw a3, 4(a2)
	{0x00000100, 0xfe000613}, // addi a2, x0, 0xfe0
	{0x00000104, 0xfffef6b7}, // lui a3, 0xfffef
	{0x00000108, 0x00d66633}, // or a2, a2, a3
	{0x0000010c, 0x02b62023}, // sw a1, 32(a2)
	{0x00000110, 0xffff0637}, // lui a2, 0xffff0
	{0x00000114, 0x00062683}, // lw a3, 0(a2)
	{0xffff0000, 0x000000ff}, // A[0]
	{0xffff0004, 0x00000010}  // A[1]
};

std::multimap<uint32_t, uint32_t> instructionsCSRECALL = {
	{0x00000000, 0x00100613}, // addi a2, x0, 0x1
	{0x00000004, 0x00300693}, // addi a3, x0, 0x3
	{0x00000008, 0x00d60663}, // bne a2, a3, end_loop
	{0x0000000c, 0x00160613}, // addi a2, a2, 0x1
	{0x00000010, 0xffff806f}  // jal x0, loop
};

std::multimap<uint32_t, uint32_t> instructionsBNE = {
	{0x00000000, 0x00268693}, // addi a3, a3, 0x2
	{0x00000004, 0x000080ef}, // jal x0, jmp
	{0x00000008, 0x00300693}, // addi a3, x0, 0x3
	{0x0000000c, 0x00160613}, // addi a2, a2, 0x1 (loop)
	{0x00000010, 0xfed61e63}  // bne a2, a3, loop
};

std::multimap<uint32_t, uint32_t> instructionsBLT = {
	{0x00000000, 0x00268693}, // addi a3, a3, 0x2
	{0x00000004, 0x000080ef}, // jal x0, jmp
	{0x00000008, 0x00300693}, // addi a3, x0, 0x3
	{0x0000000c, 0x00160613}, // addi a2, a2, 0x1 (loop)
	{0x00000010, 0xfed64e63}  // blt a2, a3, loop
};

std::multimap<uint32_t, uint32_t> instructionsBGE = {
	{0x00000000, 0x00300693}, // addi a3, x0, 0x3
	{0x00000004, 0x00000613}, // addi a2, a2, 0x0
	{0x00000008, 0xfec6de63}  // bge a3, a2, -4
};

std::multimap<uint32_t, uint32_t> instructionsBLTU1 = {
	{0x00000000, 0x00300693}, // addi a3, x0, 0x3
	// {0x00000004, 0x00d06663}, // bltu x0, a3, +8
	// {0x00000004, 0xfed06e63}, // blut x0, a3, -4
	{0x00000004, 0x80000737},  // lui a4, 0x80000
	{0x00000008, 0xfed76e63} // bltu a4, a3, -4
};

int main(int argc, char *argv[])
{
	// Start simulation
	int debug = 0;
	if (argc == 2)
	{
		debug = (atoi(argv[1]) == 1) ? 1 : 0;
	}

	CPU t89(instructionsBLTU1, debug);
	if (t89.Construct(200, 200, 2, 2))
		t89.Start();

	exit(EXIT_SUCCESS);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char** instr_type = {
    "lui",
    "auipc",
    "jal",
    "jalr",
    "beq",
    "bne",
    "blt",
    "bge",
    "bltu",
    "bgeu",
    "lb",
    "lw",
    "lbu",
    "lhu",
    "sb",
    "sh",
    "sw",
    "addi",
    "slti",
    "sltiu",
    "xori",
    "ori",
    "andi",
    "slli",
    "srli",
    "srai",
    "add",
    "sub",
    "sll",
    "slt",
    "sltu"
    "xor",
    "srl",
    "sra",
    "or",
    "and",
    "ecall",
    "csrrw",
    "csrrs"
};

int main(void) {
    printf("Hello world\n");
    exit(1);
}
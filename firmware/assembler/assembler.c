#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#define INSTRUCTION_LENGTH 10

char** instructions;
int num_instructions;

void initialize(FILE* fd) {
    size_t buf_size = INSTRUCTION_LENGTH;
    char* buffer = (char*)malloc(buf_size * sizeof(char));

    while (getline(&buffer, &buf_size, fd) != -1)
        num_instructions++;
    rewind(fd);

    instructions = (char**)malloc(num_instructions * sizeof(char*));
    for (int i = 0; i < num_instructions; i++) {
        instructions[i] = (char*)malloc(INSTRUCTION_LENGTH * sizeof(char));
        getline(&buffer, &buf_size, fd);
        buffer[strlen(buffer)-1] = '\0';
        strncpy(instructions[i], buffer, INSTRUCTION_LENGTH - 1);
    }

    for (int i = 0; i < num_instructions; i++) {
        printf("%s\n", instructions[i]);
    }
}

int main(int argc, char** argv) {
    // Argument Validation
    if (argc == 1) {
        printf("Error: asm file not found\n");
        exit(EXIT_FAILURE);
    } else if (argc == 2) {
        FILE* fd = fopen("instructions.txt", "r");
        if (fd == NULL) {
            printf("Error: could not open %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        initialize(fd);
    }
    // exit(1);
}
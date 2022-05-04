#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#define INSTRUCTION_LENGTH 10

enum Instruction_Type {
    UPPER_IMMEDIATE,
    JUMP,
    BRANCH,
    LOAD,
    STORE,
    IMMEDIATE,
    REGISTER,
    CSR
};

struct Instruction {
    enum Instruction_Type instr_type;
    int* instr_fields;
};

struct reg_pair {
    char* name;
    char* pseudo_name;
};

struct Architecture {
    char** instruction_types;
    struct reg_pair* registers;  
    int num_instructions;
    int num_registers;
};

struct Architecture risc;

void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ' || *d == '\t' || *d == '\n') {
            ++d;
        }
    } while (*s++ = *d++);
}

void initialize() {
    FILE* fd = fopen("./arch-spec/instructions.txt", "r");
    if (fd == NULL) {
        printf("Error: could not open instructions.txt");
        exit(EXIT_FAILURE);
    }
    risc.num_instructions = 0;
    size_t buf_size = INSTRUCTION_LENGTH;
    char* buffer = (char*)malloc(buf_size * sizeof(char));

    // Read Instructions types
    while (getline(&buffer, &buf_size, fd) != -1)
        risc.num_instructions++;
    rewind(fd);

    risc.instruction_types = (char**)malloc(risc.num_instructions * sizeof(char*));
    for (int i = 0; i < risc.num_instructions; i++) {
        risc.instruction_types[i] = (char*)malloc(INSTRUCTION_LENGTH * sizeof(char));
        getline(&buffer, &buf_size, fd);
        remove_spaces(buffer);
        strcpy(risc.instruction_types[i], buffer);
    }

    // Read Register names
    fd = fopen("./arch-spec/registers.txt", "r");
    if (fd == NULL) {
        printf("Error: Could not open registers.txt\n");
        exit(EXIT_FAILURE);
    }

    risc.num_registers = 0;
    while (getline(&buffer, &buf_size, fd) != -1)
        risc.num_registers++;
    rewind(fd);

    risc.registers = (struct reg_pair*)malloc(risc.num_registers * sizeof(struct reg_pair));
    char* name = (char*)malloc(strlen(buffer) * sizeof(char));
    char* pseudo_name = (char*)malloc(strlen(buffer) * sizeof(char));
    char* delim; // pointer to comma
    for (int i = 0; i < risc.num_registers; i++) {
        // Get name, pseudo-name pair
        getline(&buffer, &buf_size, fd);
        remove_spaces(buffer);
        delim = strchr(buffer , ',');
        risc.registers[i].name = (char*)malloc(strlen(buffer)*sizeof(char));
        risc.registers[i].pseudo_name = (char*)malloc(strlen(buffer)*sizeof(char));
        strncpy(risc.registers[i].name, buffer, (int)(delim - buffer));
        strcpy(risc.registers[i].pseudo_name, ++delim);
    }
}

void tokenize(char* asm_file_name) {
    // char* line
    char *buffer = (char*)malloc(INSTRUCTION_LENGTH * sizeof(char));
    size_t buffer_size = 500;
    char *line = (char*)malloc(500 * sizeof(char));
    FILE *fd = fopen(asm_file_name, "r");
    if (fd == NULL) {
        printf("Error: could not open %s", asm_file_name);
        exit(EXIT_FAILURE);
    }

    // Read each line
    while(getline(&buffer, &buffer_size, fd) != -1) {
        char* comment = strchr(buffer, ';');
        if (comment != NULL) {
            // Remove Comments
            int index = comment - buffer;
            printf("Before: %s\n", buffer);
            strncpy(line, &buffer[0], index);
            printf("After: %s\n", line);
        } else {
            // No comments
            strncpy(line, &buffer[0], strlen(buffer));
        }

        // Determine if instruction, directive, section, label
        if (isInstruction(buffer)) {
            
        }
    }
}

int main(int argc, char** argv) {
    // Argument Validation
    if (argc == 1) {
        printf("Error: asm file not found\n");
        exit(EXIT_FAILURE);
    }

    // Initialize register and instruction names
    initialize();

    // Tokenize instructions
    tokenize(argv[1]);

    exit(EXIT_SUCCESS);
}
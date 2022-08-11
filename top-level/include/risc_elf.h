#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "Components.h"

typedef uint32_t Elf32_Addr; // Unsigned program address
typedef uint16_t Elf32_Half; // Unsigned medium integer
typedef uint32_t Elf32_Off; // Unsigned file offset
typedef uint32_t Elf32_Sword; // Signed large integer
typedef uint32_t Elf32_Word; // Unsigned large integer

#define MAGIC (uint32_t)0x7F454c46

#define PT_LOAD 1 // Load Program Entry if PT_LOAD

#define DEBUG

#define EI_MAG0     0
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6
#define EI_PAD      7
#define EI_NIDENT   16

#define ELFCLASS32  1
#define ELFDATA2LSB 1
#define EV_CURRENT  1
#define ET_EXEC     2
#define EM_RISCV    0xf3

struct ELF_Header {
    unsigned char   ident[EI_NIDENT]; // Magic Number
    Elf32_Half      type; // Executable - 2
    Elf32_Half      machine; //
    Elf32_Word      version;
    Elf32_Addr      entry;
    Elf32_Off       phoff;
    Elf32_Off       shoff;
    Elf32_Word      flags;
    Elf32_Half      ehsize;
    Elf32_Half      phentsize;
    Elf32_Half      phnum;
    Elf32_Half      shentsize;
    Elf32_Half      shnum;
    Elf32_Half      shstrndx;  
};

struct ELF_File_Information {
    uint8_t* elf_data;
    unsigned int elf_size;
};

struct ELF_Section_Header {
    Elf32_Word      name;
    Elf32_Word      type;
    Elf32_Word      flags;
    Elf32_Addr      addr;
    Elf32_Off       offset;
    Elf32_Word      size;
    Elf32_Word      link;
    Elf32_Word      info;
    Elf32_Word      addraalign;
    Elf32_Word      entsize;
};

struct ELF_Program_Header {
    Elf32_Word      type;
    Elf32_Off       offset;
    Elf32_Addr      vaddr;
    Elf32_Addr      paddr;
    Elf32_Word      filesz;
    Elf32_Word      memsz;
    Elf32_Word      flags;
    Elf32_Word      align;
};

class ELF_Parse {
private:
    struct ELF_Header* elf_header_info;
    struct ELF_File_Information* elf_file_info;
    struct ELF_Program_Header* elf_program_header;
    bool elf_allocate_structures();
    bool is_legal_elf();
    const char* file_name;
    
public:
    ELF_Parse(const char*);
    bool elf_init_headers();
    bool elf_load_sections(Memory*);
};


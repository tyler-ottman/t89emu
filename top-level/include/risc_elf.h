#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <utility>

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

// Symbol Type
#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_LOPROC  13
#define STT_HIPROC  15
#define ELF32_ST_TYPE(i) ((i)&0xf) 

// Program Section Flags
#define PF_X        0x1 // Execute  
#define PF_W        0x2 // Write
#define PF_R        0x4 // Read

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

struct ELF_Symbol {
    Elf32_Word name;
    Elf32_Addr value;
    Elf32_Word size;
    uint8_t info;
    uint8_t other;
    Elf32_Half shndx;
};

struct ELF_File_Information {
    uint8_t* elf_data;
    unsigned int elf_size;
};

struct Disassembled_Entry {
    bool is_instruction; // Line if either Function/Assembly name or Instruction
    Elf32_Word address;
    std::string line;
};

class ELF_Parse {
private:
    bool elf_init_headers();

    const ELF_Section_Header* get_section_header(const char*);
    const ELF_Program_Header* get_program_header(int);
    std::pair<Elf32_Addr, std::string>* find_symbol_at_address(Elf32_Addr);
    std::string disassemble_instruction(Elf32_Addr, Elf32_Word);
    std::string get_csr_name(int);

    // Elf Header Information
    const struct ELF_Header* elf_header_info;
    const struct ELF_Program_Header* elf_program_header;
    const struct ELF_Section_Header* elf_section_header;
    
    // ELF File Information
    struct ELF_File_Information* elf_file_info;
    const char* file_name;

    std::vector<std::pair<Elf32_Addr, std::string>> symbols; // Symbol - Address, Name
    std::vector<const struct ELF_Program_Header*> executable_sections; // Section - Pointer to beginning of section, section size

    std::vector<struct Disassembled_Entry> disassembled_code;
    
    // uint8_t* rom_image; // ROM/RAM initially flashedd to ROM
public:
    ELF_Parse(const char*);
    ~ELF_Parse();
    bool elf_flash_sections();
    bool generate_disassembled_text();
    Elf32_Addr get_entry_pc();
    std::vector<struct Disassembled_Entry> get_disassembled_code();
    uint8_t* get_rom_image();

    std::vector<uint8_t> flash_image;
    std::vector<uint8_t> rom_image; // ROM section
    std::vector<uint8_t> ram_image; // RAM section
    uint8_t* raw_image; // This is "flashed" to ROM in the emulator
    uint32_t rom_start = 0;
    uint32_t rom_size = 0;
    uint32_t ram_start = 0;
    uint32_t ram_size = 0;
};
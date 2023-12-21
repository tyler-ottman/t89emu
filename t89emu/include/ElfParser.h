#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <vector>

#include "Architecture.h"
#include "ImmediateGenerator.h"
#include "RomMemoryDevice.h"

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

struct ElfHeader {
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

struct ElfSectionHeader {
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

struct ElfProgramHeader {
    Elf32_Word      type;
    Elf32_Off       offset;
    Elf32_Addr      vaddr;
    Elf32_Addr      paddr;
    Elf32_Word      filesz;
    Elf32_Word      memsz;
    Elf32_Word      flags;
    Elf32_Word      align;
};

struct ElfSymbol {
    Elf32_Word name;
    Elf32_Addr value;
    Elf32_Word size;
    uint8_t info;
    uint8_t other;
    Elf32_Half shndx;
};

struct ElfFileInformation {
    uint8_t *elfData;
    unsigned int elfSize;
};

struct DisassembledEntry {
    bool isInstruction; // Line is either Function/Assembly name or Instruction
    Elf32_Word address;
    std::string line;
};

#ifndef ELF_PARSE_H
#define ELF_PARSE_H

class ElfParser {
public:
    ElfParser(const char *path);
    ~ElfParser();
    
    void flashRom(RomMemoryDevice *romDevice);
    
    std::vector<struct DisassembledEntry> &getDisassembledCode(void);
    Elf32_Addr getEntryPc(void);
    uint32_t getRamStart(void);
    uint32_t getRomStart(void);

    bool isDebuggable(void);
    
protected:
    // Initialize ELF Parsing
    bool initStructures(const char *path);
    bool generateDisassembledCode(void);

    const ElfSectionHeader *getSectionHeader(const char *name);
    const ElfProgramHeader *getProgramHeader(int nentry);
    std::pair<Elf32_Addr, std::string> *getSymbolAtAddress(Elf32_Addr addr);
    std::string getInstructionStr(Elf32_Addr addr, Elf32_Word instruction);
    std::string getCsrName(int csrAddr);

    // ELF Header Information
    const struct ElfHeader *elfHeaderInfo;
    
    // ELF File Information
    struct ElfFileInformation *elfFileInfo;

    // ROM/RAM Information
    const struct ElfProgramHeader *ramHeader;
    const struct ElfProgramHeader *romHeader;

    // List of all executable sections within ELF file
    std::vector<const struct ElfProgramHeader *> executableSections;

    // list of (Address, Name) pairs generated from symbol table for GUI
    std::vector<std::pair<Elf32_Addr, std::string>> symbols;

    // List of (Address, Instruction) pairs for GUI
    std::vector<struct DisassembledEntry> disassembledCode;
};

#endif // ELF_PARSE_H
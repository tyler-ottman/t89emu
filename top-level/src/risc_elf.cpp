#include "risc_elf.h"

ELF_Parse::ELF_Parse(const char* filepath) {
	file_name = filepath;
	if(!elf_allocate_structures()) {std::cerr << "Error: ELF initialization failed\n"; exit(EXIT_FAILURE);}
	if(!elf_init_headers()) {std::cerr << "Error: Invalid ELF file.\n"; exit(EXIT_FAILURE);}
}

bool ELF_Parse::elf_allocate_structures() {
	elf_header_info = (const struct ELF_Header*) malloc(sizeof(struct ELF_Header));
	elf_section_header = (const struct ELF_Section_Header*) malloc(sizeof(struct ELF_Section_Header));
	elf_program_header = (const struct ELF_Program_Header*) malloc(sizeof(struct ELF_Program_Header));
	elf_file_info = (struct ELF_File_Information*) malloc(sizeof(struct ELF_File_Information));
	return ((elf_header_info != NULL) && (elf_file_info != NULL) && (elf_program_header != NULL) && (elf_section_header != NULL));
}

bool ELF_Parse::is_legal_elf() {
	if ((elf_header_info->ident[EI_MAG0] != 0x7f) || // Verify Magic
		(elf_header_info->ident[EI_MAG1] != 'E') ||
		(elf_header_info->ident[EI_MAG2] != 'L') ||
		(elf_header_info->ident[EI_MAG3] != 'F') ||
		(elf_header_info->ident[EI_CLASS] != ELFCLASS32) || // Verify Size of Architecture (32-bit)
		(elf_header_info->ident[EI_DATA] != ELFDATA2LSB) || // Emulator assumes ELF file is little endian (fix in future to handle big endian?)
		(elf_header_info->ident[EI_VERSION] != EV_CURRENT) ||
		(elf_header_info->type != ET_EXEC) || // Verify Object File type (Must be executable)
		(elf_header_info->machine != EM_RISCV) || // Verify Target Instruction Set Architecture
		(elf_header_info->version != EV_CURRENT)) {return false;} // Verify ELF Version 
	return true;
}

bool ELF_Parse::elf_init_headers() {
	FILE* fp = fopen(file_name, "rb");
	if (fp == NULL) {
		std::cerr << "Error: could not open " << file_name << "\n";
		return false;	
	}
	
	// Determine size of elf file, copy file data to memory	
	fseek(fp, 0, SEEK_END);
	elf_file_info->elf_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (!elf_file_info->elf_size) {
		fclose(fp);
		return false;
	}

	elf_file_info->elf_data = (u_int8_t*) malloc(elf_file_info->elf_size * sizeof(uint8_t));
	size_t index = fread((void*)elf_file_info->elf_data, 1, elf_file_info->elf_size, fp);
	fclose(fp);
	
	if (index != elf_file_info->elf_size) {
		free(elf_file_info->elf_data);
		return false;
	}

	// Load ELF Header information to struct
	elf_header_info = (struct ELF_Header*)elf_file_info->elf_data;

	return (is_legal_elf());
}

// Return struct pointer to desired section given section name
const ELF_Section_Header* ELF_Parse::get_section_header(const char* name) {
	// Must use string table section to read name of other sections
	for (int idx = 0; idx < elf_header_info->shnum; idx++) {
		// Current Section header idx
		const struct ELF_Section_Header* s_hdr = (const struct ELF_Section_Header*)(elf_file_info->elf_data + elf_header_info->shoff + idx * elf_header_info->shentsize);
		
		// shstrndx header (contains names of all sections), used to get name of section header idx
		const struct ELF_Section_Header* shstrndx_hdr = (const struct ELF_Section_Header*)(elf_file_info->elf_data + elf_header_info->shoff + elf_header_info->shstrndx * elf_header_info->shentsize);
		const char* section_name = (const char *)(elf_file_info->elf_data + shstrndx_hdr->offset + s_hdr->name);
		
		if (!strncmp(section_name, name, strlen(name))) {return s_hdr;}
	}
	return nullptr;
}

// Return struct pointer to desired section given nth entry in program header table
const ELF_Program_Header* ELF_Parse::get_program_header(int nentry) {
	if ((nentry < 0) || ((nentry >= elf_header_info->phnum))) {return nullptr;}
	const struct ELF_Program_Header* p_hdr = (const struct ELF_Program_Header*)(elf_file_info->elf_data + elf_header_info->phoff + nentry * elf_header_info->phentsize);
	return p_hdr;
}

// Flash all loadable sections as contiguous byte array to ROM
bool ELF_Parse::elf_load_sections(Memory* dram) {
	uint32_t rom_addr = 0;
	bool is_executable = 0; // Flag used for disassembler

	// Iterate through program table entries
	int p_num = elf_header_info->phnum;
	for (int idx = 0; idx < p_num; idx++) {
		const struct ELF_Program_Header* p_hdr = get_program_header(idx);

		// Determine if section should be loaded to emulator memory
		if (p_hdr->type != PT_LOAD) {continue;}

		// Starting Address / Size of section
		uint32_t section_size = (p_hdr->memsz < p_hdr->filesz) ? p_hdr->memsz : p_hdr->filesz;
		uint32_t address_start = p_hdr->paddr; // Starting address of program section

		// Flash section to ROM
		for (size_t jdx = 0; jdx < section_size; jdx++) {
			uint8_t* temp = (uint8_t*)(elf_file_info->elf_data + p_hdr->offset);
			// dram->write()
		}
		printf("\n");

		// If seciton if executable, add to disassembler
		if ((p_hdr->flags & 1) == PF_X) {
			add_disassembled_section((uint8_t*)(elf_file_info->elf_data + p_hdr->offset), section_size);
		}
	}

	return true;
}

void ELF_Parse::generate_disassembled_text() {
	// Use String/Symble Headers for reference
	const struct ELF_Section_Header* strtab_hdr = get_section_header(".strtab");
	const struct ELF_Section_Header* symtab_hdr = get_section_header(".symtab");
	
	// Traverse through Symbol Table to determine which symbols to use
	const struct ELF_Symbol* start_sym = (const struct ELF_Symbol*)(elf_file_info->elf_data + symtab_hdr->offset);
	const struct ELF_Symbol* end_sym = (const struct ELF_Symbol*)(elf_file_info->elf_data + symtab_hdr->offset + symtab_hdr->size);
	printf("\n# of Symbols %d\n", (int)((end_sym - start_sym)));

	const char* strtab_str = (const char*)(elf_file_info->elf_data + strtab_hdr->offset);

	// Find relevant symbols for disassembler
	int idx = 0;
	while (start_sym < end_sym) {
		const char* symbol_name = strtab_str + start_sym->name;
		printf("%02d: %s\n",idx++, symbol_name);
		start_sym++;
	}
	printf("\n");
}


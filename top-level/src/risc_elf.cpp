#include "risc_elf.h"

ELF_Parse::ELF_Parse(const char* filepath) {
	file_name = filepath;
	if(!elf_allocate_structures()) {std::cerr << "Error: ELF initialization failed\n"; exit(EXIT_FAILURE);}
}

bool ELF_Parse::elf_allocate_structures() {
	elf_header_info = (struct ELF_Header*) malloc(sizeof(struct ELF_Header));
	elf_file_info = (struct ELF_File_Information*) malloc(sizeof(struct ELF_File_Information));
	elf_program_header = (struct ELF_Program_Header*) malloc(sizeof(struct ELF_Program_Header));
	return ((elf_header_info != NULL) && (elf_file_info != NULL) && (elf_program_header != NULL));
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

#ifndef DEBUG
	std::cout << "Entry: " << std::hex << elf_header_info->entry <<
				 "\nProgram Header Table Start: " << elf_header_info->phoff <<
				 "\nSection Header Table Start: " << elf_header_info->shoff <<
				 "\nFlags: " << elf_header_info->flags <<
				 "\nHeader size: " << elf_header_info->ehsize <<
				 "\nSize of Program Header Table entry: " << elf_header_info->phentsize <<
				 "\n# of program header table entries: " << elf_header_info->phnum <<
				 "\nSize of Section Header Table entry: " << elf_header_info->shentsize <<
				 "\n# of section header table entries: " << elf_header_info->shnum <<
				 "\nIndex of Section Table containing section names: " << elf_header_info->shstrndx << "\n";


	// Program Header Table Debug
	std::cout << "\nProgram Header Table";
	int idx;
	struct ELF_Program_Header* p_entry = (struct ELF_Program_Header*)malloc(sizeof(struct ELF_Program_Header));
	for (idx = 0; idx < elf_header_info->phnum; idx++) {
		std::cout << "\nENTRY: " << idx;
		p_entry = (struct ELF_Program_Header*)(elf_file_info->elf_data + elf_header_info->phoff + idx * elf_header_info->phentsize);
		std::cout << "\np_type: " << p_entry->type <<
				 "\np_offset: " << p_entry->offset <<
				 "\np_vaddr: " << p_entry->vaddr <<
				 "\np_paddr: " << p_entry->paddr <<
				 "\np_filesz: " << p_entry->filesz <<
				 "\np_memsz: " << p_entry->memsz <<
				 "\np_flags: " << p_entry->flags <<
				 "\np_align: " << p_entry->align << "\n";
	}
#endif
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

	// Load ELF Program Header information to struct
	elf_program_header = (struct ELF_Program_Header*)(elf_file_info->elf_data + elf_header_info->phoff);

	return (is_legal_elf());
}

bool ELF_Parse::elf_load_sections(Memory* dram) {
	uint32_t rom_addr = 0;

	// Iterate through program table entries
	int p_num = elf_header_info->phnum;
	for (int idx = 0; idx < p_num; idx++) {
		const struct ELF_Program_Header* p_hdr = (const struct ELF_Program_Header*)(elf_file_info->elf_data + elf_header_info->phoff + idx * elf_header_info->phentsize);

		// Determine if section should be loaded to emulator
		if (p_hdr->type != PT_LOAD) {continue;}

		// Copy section to memory
		uint32_t section_size = (p_hdr->memsz < p_hdr->filesz) ? p_hdr->memsz : p_hdr->filesz;
		uint32_t address_start = p_hdr->paddr; // Starting address of program section

		// Flash section to ROM		
		for (size_t jdx = 0; jdx < section_size / 4; jdx++) {
			uint32_t* temp = (uint32_t*)(elf_file_info->elf_data + p_hdr->offset);
			printf("%08x ", temp[jdx]);
			// printf("%02x ", word);
		}
		printf("\n");
	}
	return true;
}
#include "risc_elf.h"

ELF_Parse::ELF_Parse(const char* filepath) {
	file_name = filepath;
	elf_file_info = new ELF_File_Information;
	if((elf_file_info == NULL) || !elf_init_headers()) {
		std::cerr << "Error: ELF initialization failed\n";
		exit(EXIT_FAILURE);
	}
}

ELF_Parse::~ELF_Parse() {
	delete elf_file_info;
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
	if (elf_file_info->elf_data == nullptr) {return false;}
	size_t index = fread((void*)elf_file_info->elf_data, 1, elf_file_info->elf_size, fp);
	fclose(fp);
	
	if (index != elf_file_info->elf_size) {
		free(elf_file_info);
		return false;
	}

	// Load ELF Header information to struct
	elf_header_info = (struct ELF_Header*)elf_file_info->elf_data;

	// Verify ELF format
	return ((elf_header_info->ident[EI_MAG0] == 0x7f) && // Verify Magic
			(elf_header_info->ident[EI_MAG1] == 'E') &&
			(elf_header_info->ident[EI_MAG2] == 'L') &&
			(elf_header_info->ident[EI_MAG3] == 'F') &&
			(elf_header_info->ident[EI_CLASS] == ELFCLASS32) && // Verify Size of Architecture (32-bit)
			(elf_header_info->ident[EI_DATA] == ELFDATA2LSB) && // Emulator assumes ELF file is little endian (fix in future to handle big endian?)
			(elf_header_info->ident[EI_VERSION] == EV_CURRENT) &&
			(elf_header_info->type == ET_EXEC) && // Verify Object File type (Must be executable)
			(elf_header_info->machine == EM_RISCV) && // Verify Target Instruction Set Architecture
			(elf_header_info->version == EV_CURRENT));
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

// Flash all loadable sections as one contiguous byte array to ROM
bool ELF_Parse::elf_flash_sections(Memory* dram) {
	std::vector<uint8_t> flash_image;

	// Iterate through program table entries
	int p_num = elf_header_info->phnum;
	for (int idx = 0; idx < p_num; idx++) {
		const struct ELF_Program_Header* p_hdr = get_program_header(idx);

		// Determine if section should be loaded to emulator memory
		if (p_hdr->type != PT_LOAD) {continue;}

		// Size of section
		Elf32_Word section_size = (p_hdr->memsz < p_hdr->filesz) ? p_hdr->memsz : p_hdr->filesz;

		// Flash section to ROM
		for (size_t jdx = 0; jdx < section_size; jdx++) {
			uint8_t* data = (uint8_t*)(elf_file_info->elf_data + p_hdr->offset + jdx);
			flash_image.push_back(*data);
		}

		// If section is executable, add to disassembled text 
		if ((p_hdr->flags & 1) == PF_X) {
			executable_sections.push_back(p_hdr);
		}
	}

	// printf("Full image:\n");
	// for (Elf32_Addr addr = 0; addr < flash_image.size(); addr++) {
	// 	// dram->write(addr, flash_image.at(addr), BYTE);
	// 	printf("%02x ", flash_image.at(addr));
	// }
	return true;
}

std::pair<Elf32_Addr, std::string>* ELF_Parse::find_symbol_at_address(Elf32_Addr addr) {
	for (auto &symbol : symbols) {
		if (symbol.first == addr) {
			return &symbol;
		}
	}
	return nullptr;
}

void ELF_Parse::generate_disassembled_text() {
	// Use String/Symble Headers for reference
	const struct ELF_Section_Header* strtab_hdr = get_section_header(".strtab");
	const struct ELF_Section_Header* symtab_hdr = get_section_header(".symtab");
	
	// Traverse through Symbol Table to determine which symbols to use
	const struct ELF_Symbol* start_sym = (const struct ELF_Symbol*)(elf_file_info->elf_data + symtab_hdr->offset);
	const struct ELF_Symbol* end_sym = (const struct ELF_Symbol*)(elf_file_info->elf_data + symtab_hdr->offset + symtab_hdr->size);
	// printf("\n# of Symbols %d\n", (int)((end_sym - start_sym)));

	const char* strtab_str = (const char*)(elf_file_info->elf_data + strtab_hdr->offset);

	// Load Symbols (address)
	for (start_sym = start_sym; start_sym < end_sym; start_sym++) {
		const char* symbol_name = strtab_str + start_sym->name;
		const Elf32_Addr symbol_addr = start_sym->value;
		
		// Discard "$x" symbols and UND symbol
		if (!strcmp("$x", symbol_name) || !strlen(symbol_name)){continue;}

		// Disard certain symbol types
		switch(ELF32_ST_TYPE(start_sym->info)) {
		case STT_OBJECT :
		case STT_SECTION :
		case STT_FILE :
			continue;
		default :
			symbols.push_back(std::make_pair(symbol_addr, symbol_name));
		}

		if (!strcmp("$x", symbol_name) || (strlen(symbol_name) == 0) || (start_sym->info == STT_FILE) || (start_sym->info == STT_OBJECT)) {
			continue;
		}

		// printf("%08x: %s\n", symbol_addr, symbol_name);
	}
	printf("\n");

	// Merge Symbols and Instructions
	for (const auto &section: executable_sections) {
		// Starting Address / Size of section
		Elf32_Word section_size = (section->memsz < section->filesz) ? section->memsz : section->filesz;
		Elf32_Addr address_start = section->paddr; // Starting address of program section

		// Add current executable section to text
		for (Elf32_Addr idx = 0; idx < section_size; idx += 4) {
			std::pair<Elf32_Addr, std::string>* addr_symb = find_symbol_at_address(address_start + idx);
			if (addr_symb != nullptr) {
				printf("%s\n", addr_symb->second.c_str());
			}

			Elf32_Word instruction = *((uint32_t*)(elf_file_info->elf_data + section->offset + idx));
			printf("%08x\n", instruction);
		}
	}
	std::cout << std::endl;
	// for (const auto &symbol : symbols) {
	// 	printf("%08x: %s\n", symbol.first, symbol.second.c_str());
	// }
	// If seciton if executable, add to disassembler
	// if ((p_hdr->flags & 1) == PF_X) {
	// 	add_disassembled_section((uint8_t*)(elf_file_info->elf_data + p_hdr->offset), section_size);
	// }

}
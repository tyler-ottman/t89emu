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
	delete rom_image;
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
bool ELF_Parse::elf_flash_sections() {
	// Iterate through program table entries
	int p_num = elf_header_info->phnum;
	for (int idx = 0; idx < p_num; idx++) {
		const struct ELF_Program_Header* p_hdr = get_program_header(idx);

		// Determine if section should be loaded to emulator memory
		if (p_hdr->type != PT_LOAD) {continue;}

		// Size of section
		Elf32_Word section_size = (p_hdr->memsz < p_hdr->filesz) ? p_hdr->memsz : p_hdr->filesz;

		// If seciton is readable/write, mark as RAM
		if ((p_hdr->flags & PF_R) && (p_hdr->flags & PF_W)) {
			ram_start = p_hdr->paddr;
			ram_size = section_size;
		} else if ((p_hdr->flags & PF_R) && (p_hdr->flags & PF_X)) {
			rom_start = p_hdr->paddr;
			rom_size = section_size;
		}

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

	// for (auto const &entry : flash_image) {
	// 	printf("%02x ", entry);
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

uint8_t* ELF_Parse::get_rom_image() {
	uint32_t flash_size = flash_image.size();
	// rom_image = new uint8_t(flash_size);
	rom_image = (uint8_t*)malloc(flash_size * sizeof(uint8_t));
	for (uint32_t idx = 0; idx < flash_size; idx++) {
		rom_image[idx] = flash_image.at(idx);
	}
	return rom_image;
}

bool ELF_Parse::generate_disassembled_text() {
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

	}
	// Merge Symbols and Instructions
	for (const auto &section: executable_sections) {
		// Starting Address / Size of section
		Elf32_Word section_size = (section->memsz < section->filesz) ? section->memsz : section->filesz;
		Elf32_Addr address_start = section->paddr; // Starting address of program section

		// Add current executable section to text
		for (Elf32_Addr idx = 0; idx < section_size; idx += 4) {
			Elf32_Addr cur_addr = address_start + idx;
			struct Disassembled_Entry disassembled_line; // Used for Disassembler in GUI
			
			std::pair<Elf32_Addr, std::string>* addr_symb = find_symbol_at_address(cur_addr);
			if (addr_symb != nullptr) { // Instruction at current address also has a symbol (function or assembly routine name)
				disassembled_line.is_instruction = false;
				disassembled_line.address = cur_addr;
				disassembled_line.line = "<" + addr_symb->second + ">:";
				disassembled_code.push_back(disassembled_line);
			}

			Elf32_Word instruction = *((uint32_t*)(elf_file_info->elf_data + section->offset + idx));
			disassembled_line.is_instruction = true;
			disassembled_line.address = cur_addr;
			disassembled_line.line = disassemble_instruction(cur_addr, instruction);
			disassembled_code.push_back(disassembled_line);
		}
	}
	return true;
}

std::vector<struct Disassembled_Entry> ELF_Parse::get_disassembled_code() {
	return disassembled_code;
}

Elf32_Addr ELF_Parse::get_entry_pc() {
	return elf_header_info->entry;
}

std::string ELF_Parse::disassemble_instruction(Elf32_Addr addr, Elf32_Word instruction) {
	const std::vector<std::string> instruction_names = {
		"lui", "auipc", "jal", "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu",
		"lb", "lh", "lw", "lbu", "lhu", "sb", "sh", "sw", "addi"
	};
	const std::vector<std::string> register_names = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

	const std::vector<std::string> branch_isntructions = {"beq", "bne", "nan", "nan", "blt", "bge", "bltu", "bgeu"};
	const std::vector<std::string> load_instructions = {"lb", "lh", "lw", "nan", "lbu", "lhu"};
	const std::vector<std::string> store_instructions = {"sb", "sh", "sw"};
	const std::vector<std::string> i_instructions = {"addi", "slli", "slti", "sltiu", "xori", "nan", "ori", "andi"};
	const std::vector<std::string> srli_srai = {"srli", "srai"};
	const std::vector<std::string> r_instructions = {"nan", "sll", "slt", "sltu", "xor", "nan", "or", "and"};
	const std::vector<std::string> csr_instructions = {"nan", "csrrw", "csrrs", "csrrc"};

	ImmediateGenerator* immgen = new ImmediateGenerator;

	int opcode = instruction & 0x7f;
	uint32_t funct3 = (instruction >> 12) & 0b111;
	uint32_t funct7 = (instruction >> 25) & 0b1111111;
	uint32_t rs1 = (instruction >> 15) & 0b11111;
	uint32_t rs2 = (instruction >> 20) & 0b11111;
	uint32_t rd = (instruction >> 7) & 0b11111;
	uint32_t immediate = immgen->getImmediate(instruction);
	uint32_t csr_addr = (instruction >> 20) & 0xfff;

	char instruction_str[64];
	switch (opcode) {
	case LUI:
		immediate = (immediate >> 12) & 0xfffff;
		sprintf(instruction_str, "%-8s%s,0x%x", "lui", register_names.at(rd).c_str(), immediate);
		break;
	case AUIPC:
		immediate = (immediate >> 12) & 0xfffff;
		sprintf(instruction_str, "%-8s%s,0x%x", "auipc", register_names.at(rd).c_str(), immediate);
		break;
	case JAL:
		sprintf(instruction_str, "%-8s%s,%x", "jal", register_names.at(rd).c_str(), (immediate + addr));
		break;
	case JALR:
		sprintf(instruction_str, "%-8s%s,%d(%s)", "jalr", register_names.at(rd).c_str(), immediate, register_names.at(rs1).c_str());
		break;
	case BTYPE:
		sprintf(instruction_str, "%-8s%s,%s,%x", branch_isntructions.at(funct3).c_str(), register_names.at(rs1).c_str(), register_names.at(rs2).c_str(), (immediate + addr));
		break;
	case LOAD:
		sprintf(instruction_str, "%-8s%s,%d(%s)", load_instructions.at(funct3).c_str(), register_names.at(rd).c_str(), immediate, register_names.at(rs1).c_str());
		break;
	case STORE:
		sprintf(instruction_str, "%-8s%s,%d(%s)", store_instructions.at(funct3).c_str(), register_names.at(rs2).c_str(), immediate, register_names.at(rs1).c_str());
		break;
	case ITYPE:
		switch(funct3) {
		case 0b101:	// srai / srli
			switch(funct7) {
			case 0b0100000: sprintf(instruction_str, "%-8s%s,%s,0x%x", "srai", register_names.at(rd).c_str(), register_names.at(rs1).c_str(), immediate); break;
			case 0b0000000: sprintf(instruction_str, "%-8s%s,%s,0x%x", "srli", register_names.at(rd).c_str(), register_names.at(rs1).c_str(), immediate); break;
			}
			
			break;
		case 0b001:
			sprintf(instruction_str, "%-8s%s,%s,0x%x", "slli", register_names.at(rd).c_str(), register_names.at(rs1).c_str(), immediate);
			break;
		default:
			sprintf(instruction_str, "%-8s%s,%s,%d", i_instructions.at(funct3).c_str(), register_names.at(rd).c_str(), register_names.at(rs1).c_str(), immediate);
			break;
		}
		break;
	case RTYPE:
		switch(funct3) {
		case 0b000: // add / sub
			switch(funct7) {
			case 0b0000000: sprintf(instruction_str, "%-8s%s,%s,%s", "add", register_names.at(rd).c_str(), register_names.at(rs1).c_str(), register_names.at(rs2).c_str()); break;
			case 0b0100000: sprintf(instruction_str, "%-8s%s,%s,%s", "sub", register_names.at(rd).c_str(), register_names.at(rs1).c_str(), register_names.at(rs2).c_str()); break;
			}
			break;
		case 0b101: // srl / sra
			switch(funct7) {
			case 0b0000000: sprintf(instruction_str, "%-8s%s,%s,%s", "srl", register_names.at(rd).c_str(), register_names.at(rs1).c_str(), register_names.at(rs2).c_str()); break;
			case 0b0100000: sprintf(instruction_str, "%-8s%s,%s,%s", "sra", register_names.at(rd).c_str(), register_names.at(rs1).c_str(), register_names.at(rs2).c_str()); break;
			}
			break;
		default:
			sprintf(instruction_str, "%-8s%s,%s,%s", r_instructions.at(funct3).c_str(), register_names.at(rd).c_str(), register_names.at(rs1).c_str(), register_names.at(rs2).c_str()); break;
		}
		break;
	case PRIV:
		switch (funct3) {
		case 0b000: // ECALL / MRET
			switch (immediate) {
			case MRET_IMM:
				sprintf(instruction_str, "mret");
				break;
			case ECALL_IMM:
				sprintf(instruction_str, "ecall");
				break;
			}
			break;
		default:
			// CSRRW / CSRRS / CSRRC
			sprintf(instruction_str, "%-8s%s,%s,%s", csr_instructions.at(funct3).c_str(), register_names.at(rd).c_str(), get_csr_name(csr_addr).c_str(), register_names.at(rs1).c_str());
			break;
		}
		break;
	default:
		sprintf(instruction_str, "%08x", instruction);
	}

	delete immgen;
	// printf("%s\n", instruction_str);
	return instruction_str;
}

std::string ELF_Parse::get_csr_name(int csr_addr)
{
    switch (csr_addr) {
    case 0x0300: return "mstatus";
    case 0x0301: return "misa";
    case 0x0304: return "mie";
    case 0x0305: return "mtvec";
    case 0x0340: return "mscratch";
    case 0x0341: return "mepc";
    case 0x0342: return "mcause";
    case 0x0343: return "mtval";
    case 0x0344: return "mip";
    case 0x0b00: return "mcycle";
    case 0x0b02: return "minstret";
    case 0x0b80: return "mcycleh";
    case 0x0b82: return "minstreth";
    case 0x0f11: return "mvendorid";
    case 0x0f12: return "marchid";
    case 0x0f13: return "mimpid";
    case 0x0f14: return "mhartid";
    default: return "CSR_UNKWN";
    }
}

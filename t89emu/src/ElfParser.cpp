#include "ElfParser.h"

ElfParser::ElfParser(const char* filepath) {
	fileName = filepath;
	elfFileInfo = new ElfFileInformation;
	if(!elfFileInfo || !elfInitHeaders()) {
		std::cerr << "Error: ELF initialization failed\n";
		exit(EXIT_FAILURE);
	}
}

ElfParser::~ElfParser() {
	delete elfFileInfo;
	// delete romImage;
}

bool ElfParser::elfInitHeaders() {
	FILE *fp = fopen(fileName, "rb");
	if (fp == NULL) {
		std::cerr << "Error: could not open " << fileName << "\n";
		return false;	
	}
	
	// Determine size of elf file, copy file data to memory	
	fseek(fp, 0, SEEK_END);
	elfFileInfo->elfSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (!elfFileInfo->elfSize) {
		fclose(fp);
		return false;
	}

	elfFileInfo->elfData = (uint8_t *)malloc(elfFileInfo->elfSize * sizeof(uint8_t));
	if (elfFileInfo->elfData == nullptr) {return false;}
	size_t index = fread((void *)elfFileInfo->elfData, 1, elfFileInfo->elfSize, fp);
	fclose(fp);
	
	if (index != elfFileInfo->elfSize) {
		free(elfFileInfo);
		return false;
	}

	// Load ELF Header information to struct
	elfHeaderInfo = (struct ElfHeader *)elfFileInfo->elfData;

	// Verify ELF format
	return ((elfHeaderInfo->ident[EI_MAG0] == 0x7f) && // Verify Magic
			(elfHeaderInfo->ident[EI_MAG1] == 'E') &&
			(elfHeaderInfo->ident[EI_MAG2] == 'L') &&
			(elfHeaderInfo->ident[EI_MAG3] == 'F') &&
			(elfHeaderInfo->ident[EI_CLASS] == ELFCLASS32) && // Verify Size of Architecture (32-bit)
			(elfHeaderInfo->ident[EI_DATA] == ELFDATA2LSB) && // Emulator assumes ELF file is little endian (fix in future to handle big endian?)
			(elfHeaderInfo->ident[EI_VERSION] == EV_CURRENT) &&
			(elfHeaderInfo->type == ET_EXEC) && // Verify Object File type (Must be executable)
			(elfHeaderInfo->machine == EM_RISCV) && // Verify Target Instruction Set Architecture
			(elfHeaderInfo->version == EV_CURRENT));
}

// Return struct pointer to desired section given section name
const ElfSectionHeader *ElfParser::getSectionHeader(const char *name) {
	// Must use string table section to read name of other sections
	for (int idx = 0; idx < elfHeaderInfo->shnum; idx++) {
		// Current Section header idx
		const struct ElfSectionHeader *sHdr = (const struct ElfSectionHeader *)(elfFileInfo->elfData + elfHeaderInfo->shoff + idx * elfHeaderInfo->shentsize);
		
		// shstrndx header (contains names of all sections), used to get name of section header idx
		const struct ElfSectionHeader *shstrndxHdr = (const struct ElfSectionHeader *)(elfFileInfo->elfData + elfHeaderInfo->shoff + elfHeaderInfo->shstrndx * elfHeaderInfo->shentsize);
		const char* sectionName = (const char *)(elfFileInfo->elfData + shstrndxHdr->offset + sHdr->name);
		
		if (!strncmp(sectionName, name, strlen(name))) {return sHdr;}
	}
	return nullptr;
}

// Return struct pointer to desired section given nth entry in program header table
const ElfProgramHeader *ElfParser::getProgramHeader(int nentry) {
	if ((nentry < 0) || ((nentry >= elfHeaderInfo->phnum))) {return nullptr;}
	const struct ElfProgramHeader *pHdr = (const struct ElfProgramHeader *)(elfFileInfo->elfData + elfHeaderInfo->phoff + nentry * elfHeaderInfo->phentsize);
	return pHdr;
}

// Flash all loadable sections as one contiguous byte array to ROM
bool ElfParser::elfFlashSections() {
	// Iterate through program table entries
	int p_num = elfHeaderInfo->phnum;
	for (int idx = 0; idx < p_num; idx++) {
		const struct ElfProgramHeader *pHdr = getProgramHeader(idx);

		// Determine if section should be loaded to emulator memory
		if (pHdr->type != PT_LOAD) {continue;}

		// Size of section
		Elf32_Word sectionSize = (pHdr->memsz < pHdr->filesz) ? pHdr->memsz : pHdr->filesz;

		// If seciton is readable/write, mark as RAM
		if ((pHdr->flags & PF_R) && (pHdr->flags & PF_W)) {
			// Add section to RAM image
			ramStart = pHdr->paddr;
			ramSize = sectionSize;
			for (size_t jdx = 0; jdx < sectionSize; jdx++) {
				uint8_t *data = (uint8_t *)(elfFileInfo->elfData + pHdr->offset + jdx);
				ramImage.push_back(*data);
			}
		} else if ((pHdr->flags & PF_R) && (pHdr->flags & PF_X)) {
			// Add section to ROM image
			romStart = pHdr->paddr;
			romSize = sectionSize;
			for (size_t jdx = 0; jdx < sectionSize; jdx++) {
				uint8_t *data = (uint8_t *)(elfFileInfo->elfData + pHdr->offset + jdx);
				romImage.push_back(*data);
			}
		}

		// If section is executable, add to disassembled text 
		if ((pHdr->flags & 1) == PF_X) {
			executableSections.push_back(pHdr);
		}
	}

	// for (auto const &entry : flashImage) {
	// 	printf("%02x ", entry);
	// }
	return true;
}

std::pair<Elf32_Addr, std::string> *ElfParser::findSymbolAtAddress(Elf32_Addr addr) {
	for (auto &symbol : symbols) {
		if (symbol.first == addr) {
			return &symbol;
		}
	}
	return nullptr;
}

uint8_t *ElfParser::getRomImage() {
	// Add ROM, then RAM to flash image
	flashImage.insert(flashImage.end(), romImage.begin(), romImage.end());
	flashImage.insert(flashImage.end(), ramImage.begin(), ramImage.end());

	uint32_t flashSize = flashImage.size();
	rawImage = (uint8_t *)malloc(flashSize * sizeof(uint8_t));
	for (uint32_t idx = 0; idx < flashSize; idx++) {
		rawImage[idx] = flashImage.at(idx);
	}
	return rawImage;
}

bool ElfParser::generateDisassembledText() {
	// Use String/Symble Headers for reference
	const struct ElfSectionHeader *strtabHdr = getSectionHeader(".strtab");
	const struct ElfSectionHeader *symtabHdr = getSectionHeader(".symtab");
	
	// Traverse through Symbol Table to determine which symbols to use
	const struct ElfSymbol *startSym = (const struct ElfSymbol *)(elfFileInfo->elfData + symtabHdr->offset);
	const struct ElfSymbol* endSym = (const struct ElfSymbol *)(elfFileInfo->elfData + symtabHdr->offset + symtabHdr->size);
	// printf("\n# of Symbols %d\n", (int)((end_sym - start_sym)));

	const char *strtabStr = (const char *)(elfFileInfo->elfData + strtabHdr->offset);

	// Load Symbols (address)
	for (startSym = startSym; startSym < endSym; startSym++) {
		const char *symbolName = strtabStr + startSym->name;
		const Elf32_Addr symbolAddr = startSym->value;
		
		// Discard "$x" symbols and UND symbol
		if (!strcmp("$x", symbolName) || !strlen(symbolName)){ continue; }

		// Disard certain symbol types
		switch(ELF32_ST_TYPE(startSym->info)) {
		case STT_OBJECT :
		case STT_SECTION :
		case STT_FILE :
			continue;
		default :
			symbols.push_back(std::make_pair(symbolAddr, symbolName));
		}

		if (!strcmp("$x", symbolName) || (strlen(symbolName) == 0) || (startSym->info == STT_FILE) || (startSym->info == STT_OBJECT)) {
			continue;
		}

	}
	// Merge Symbols and Instructions
	for (const auto &section: executableSections) {
		// Starting Address / Size of section
		Elf32_Word section_size = (section->memsz < section->filesz) ? section->memsz : section->filesz;
		Elf32_Addr address_start = section->paddr; // Starting address of program section

		// Add current executable section to text
		for (Elf32_Addr idx = 0; idx < section_size; idx += 4) {
			Elf32_Addr curAddr = address_start + idx;
			struct DisassembledEntry disassembledLine; // Used for Disassembler in GUI
			
			std::pair<Elf32_Addr, std::string> *addrSymb = findSymbolAtAddress(curAddr);
			if (addrSymb != nullptr) { // Instruction at current address also has a symbol (function or assembly routine name)
				disassembledLine.isInstruction = false;
				disassembledLine.address = curAddr;
				disassembledLine.line = "<" + addrSymb->second + ">:";
				disassembledCode.push_back(disassembledLine);
			}

			Elf32_Word instruction = *((uint32_t *)(elfFileInfo->elfData + section->offset + idx));
			disassembledLine.isInstruction = true;
			disassembledLine.address = curAddr;
			disassembledLine.line = disassembleInstruction(curAddr, instruction);
			disassembledCode.push_back(disassembledLine);
		}
	}
	return true;
}

std::vector<struct DisassembledEntry> &ElfParser::getDisassembledCode() {
	return disassembledCode;
}

Elf32_Addr ElfParser::getEntryPc() {
	return elfHeaderInfo->entry;
}

std::string ElfParser::disassembleInstruction(Elf32_Addr addr, Elf32_Word instruction) {
	const std::vector<std::string> registerNames = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

	const std::vector<std::string> branchInstructions = {"beq", "bne", "nan", "nan", "blt", "bge", "bltu", "bgeu"};
	const std::vector<std::string> loadInstructions = {"lb", "lh", "lw", "nan", "lbu", "lhu"};
	const std::vector<std::string> storeInstructions = {"sb", "sh", "sw"};
	const std::vector<std::string> iInstructions = {"addi", "slli", "slti", "sltiu", "xori", "nan", "ori", "andi"};
	const std::vector<std::string> srliSrai = {"srli", "srai"};
	const std::vector<std::string> rInstructions = {"nan", "sll", "slt", "sltu", "xor", "nan", "or", "and"};
	const std::vector<std::string> csrInstructions = {"nan", "csrrw", "csrrs", "csrrc"};

	ImmediateGenerator* immgen = new ImmediateGenerator;

	int opcode = instruction & 0x7f;
	uint32_t funct3 = (instruction >> 12) & 0b111;
	uint32_t funct7 = (instruction >> 25) & 0b1111111;
	uint32_t rs1 = (instruction >> 15) & 0b11111;
	uint32_t rs2 = (instruction >> 20) & 0b11111;
	uint32_t rd = (instruction >> 7) & 0b11111;
	uint32_t immediate = immgen->getImmediate(instruction);
	uint32_t csr_addr = (instruction >> 20) & 0xfff;

	char instructionStr[64];
	switch (opcode) {
	case LUI:
		immediate = (immediate >> 12) & 0xfffff;
		sprintf(instructionStr, "%-8s%s,0x%x", "lui", registerNames.at(rd).c_str(), immediate);
		break;
	case AUIPC:
		immediate = (immediate >> 12) & 0xfffff;
		sprintf(instructionStr, "%-8s%s,0x%x", "auipc", registerNames.at(rd).c_str(), immediate);
		break;
	case JAL:
		sprintf(instructionStr, "%-8s%s,%x", "jal", registerNames.at(rd).c_str(), (immediate + addr));
		break;
	case JALR:
		sprintf(instructionStr, "%-8s%s,%d(%s)", "jalr", registerNames.at(rd).c_str(), immediate, registerNames.at(rs1).c_str());
		break;
	case BTYPE:
		sprintf(instructionStr, "%-8s%s,%s,%x", branchInstructions.at(funct3).c_str(), registerNames.at(rs1).c_str(), registerNames.at(rs2).c_str(), (immediate + addr));
		break;
	case LOAD:
		sprintf(instructionStr, "%-8s%s,%d(%s)", loadInstructions.at(funct3).c_str(), registerNames.at(rd).c_str(), immediate, registerNames.at(rs1).c_str());
		break;
	case STORE:
		sprintf(instructionStr, "%-8s%s,%d(%s)", storeInstructions.at(funct3).c_str(), registerNames.at(rs2).c_str(), immediate, registerNames.at(rs1).c_str());
		break;
	case ITYPE:
		switch(funct3) {
		case 0b101:	// srai / srli
			switch(funct7) {
			case 0b0100000: sprintf(instructionStr, "%-8s%s,%s,0x%x", "srai", registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), immediate); break;
			case 0b0000000: sprintf(instructionStr, "%-8s%s,%s,0x%x", "srli", registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), immediate); break;
			}
			
			break;
		case 0b001:
			sprintf(instructionStr, "%-8s%s,%s,0x%x", "slli", registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), immediate);
			break;
		default:
			sprintf(instructionStr, "%-8s%s,%s,%d", iInstructions.at(funct3).c_str(), registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), immediate);
			break;
		}
		break;
	case RTYPE:
		switch(funct3) {
		case 0b000: // add / sub
			switch(funct7) {
			case 0b0000000: sprintf(instructionStr, "%-8s%s,%s,%s", "add", registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), registerNames.at(rs2).c_str()); break;
			case 0b0100000: sprintf(instructionStr, "%-8s%s,%s,%s", "sub", registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), registerNames.at(rs2).c_str()); break;
			}
			break;
		case 0b101: // srl / sra
			switch(funct7) {
			case 0b0000000: sprintf(instructionStr, "%-8s%s,%s,%s", "srl", registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), registerNames.at(rs2).c_str()); break;
			case 0b0100000: sprintf(instructionStr, "%-8s%s,%s,%s", "sra", registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), registerNames.at(rs2).c_str()); break;
			}
			break;
		default:
			sprintf(instructionStr, "%-8s%s,%s,%s", rInstructions.at(funct3).c_str(), registerNames.at(rd).c_str(), registerNames.at(rs1).c_str(), registerNames.at(rs2).c_str()); break;
		}
		break;
	case PRIV:
		switch (funct3) {
		case 0b000: // ECALL / MRET
			switch (immediate) {
			case MRET_IMM:
				sprintf(instructionStr, "mret");
				break;
			case ECALL_IMM:
				sprintf(instructionStr, "ecall");
				break;
			}
			break;
		default:
			// CSRRW / CSRRS / CSRRC
			sprintf(instructionStr, "%-8s%s,%s,%s", csrInstructions.at(funct3).c_str(), registerNames.at(rd).c_str(), getCsrName(csr_addr).c_str(), registerNames.at(rs1).c_str());
			break;
		}
		break;
	default:
		sprintf(instructionStr, "%08x", instruction);
	}

	delete immgen;
	// printf("%s\n", instructionStr);
	return instructionStr;
}

std::string ElfParser::getCsrName(int csrAddr) {
    switch (csrAddr) {
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

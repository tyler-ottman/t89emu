#include "Mcu.h"

Mcu::Mcu(uint32_t romBase, uint32_t romSize, uint32_t ramBase, uint32_t ramSize,
         int debug = 0) {
    rf = new RegisterFile;
    pc = new ProgramCounter;
    csr = new Csr;
    alu = new Alu;
    aluc = new AluControlUnit;
    immgen = new ImmediateGenerator;
    mcu = new MemControlUnit;
    nextPc = new NextPc;
#ifndef BUS_EXPERIMENTAL
    bus = new Bus(romBase, romSize, ramBase, ramSize);
#else
    bus = new Bus();
    rom = new RomMemoryDevice(romBase, romSize);
    ram = new RamMemoryDevice(ramBase, ramSize);
    vram = new VideoMemoryDevice(VIDEO_BASE, VIDEO_SIZE);
    clint = new ClintMemoryDevice(CLINT_BASE, CLINT_SIZE);
    bus->addDevice(rom);
    bus->addDevice(ram);
    bus->addDevice(vram);
    bus->addDevice(clint);
#endif // BUS_EXPERIMENTAL
    trap = new Trap;
}

Mcu::~Mcu() {
    delete rf;
    delete pc;
    delete bus;
    delete csr;
    delete alu;
    delete aluc;
    delete immgen;
    delete mcu;
    delete nextPc;
    delete trap;
}

void Mcu::nextInstruction() {
    // Update Clint Device every cycle
#ifndef BUS_EXPERIMENTAL
    bus->getClintDevice()->nextCycle(csr);

    // Check for interrupts
    if (bus->getClintDevice()->checkInterrupts(csr)) {
        trap->takeTrap(csr, pc, nextPc,
                       bus->getClintDevice()->getInterruptType());
    }
#else
    clint->nextCycle(csr);

    // Check for interrupts
    if (clint->checkInterrupts(csr)) {
        trap->takeTrap(csr, pc, nextPc, clint->getInterruptType());
    }
#endif // BUS_EXPERIMENTAL

    uint32_t exceptionCode = executeInstruction();
    if (exceptionCode != STATUS_OK) {
        // Hard to emulate accurately, but previous call to
        // execute_instruction() fails because exception is thrown. In a
        // real system, the CSRs and PC will change in the same cycle, so
        // the trap bit causes the PC to switch (mux) from the faulting
        // instruction to the jump instruction in the vector table
        trap->takeTrap(csr, pc, nextPc, exceptionCode);

        // Trap phase of cycle emulated (usually means control lines switch)
        // Now execute the jump instruction in the vector table
        executeInstruction();
    }
}

Alu *Mcu::getAluModule() {
    return alu;
}

AluControlUnit *Mcu::getAluControlUnitModule() {
    return aluc;
}

Bus *Mcu::getBusModule() {
    return bus;
}

Csr *Mcu::getCsrModule() {
    return csr;
}

ImmediateGenerator *Mcu::getImmediateGeneratorModule() {
    return immgen;
}

MemControlUnit *Mcu::getMemControlUnitModule() {
    return mcu;
}

NextPc *Mcu::getNextPcModule() {
    return nextPc;
}

ProgramCounter *Mcu::getProgramCounterModule() {
    return pc;
}

RegisterFile *Mcu::getRegisterFileModule() {
    return rf;
}

Trap *Mcu::getTrapModule() {
    return trap;
}

uint32_t Mcu::executeInstruction() {
    // Fetch Stage
    uint32_t pcAddr = pc->getPc();  // Current PC
    uint32_t curInstruction;
    uint32_t exceptionCode =
        bus->read(pcAddr, WORD, &curInstruction);  // Current Instruction
    if (exceptionCode != STATUS_OK) {
        // Instruction Access fault or instruction address misaligned
        return exceptionCode;
    }

    // Decode Stage
    uint32_t opcode = curInstruction & 0b1111111;          // opcode field
    uint32_t funct3 = (curInstruction >> 12) & 0b111;      // funct3 field
    uint32_t funct7 = (curInstruction >> 25) & 0b1111111;  // funct7 field
    uint32_t rs1 = (curInstruction >> 15) & 0b11111;       // Register Source 1
    uint32_t rs2 = (curInstruction >> 20) & 0b11111;       // Register Source 2
    uint32_t rd = (curInstruction >> 7) & 0b11111;  // Register Desination
    uint32_t immediate =
        immgen->getImmediate(curInstruction);           // Instruction Immediate
    uint32_t csrAddr = (curInstruction >> 20) & 0xfff;  // CSR Address

    // Execution flow dependent on instrution type
    uint32_t accessSize;
    uint32_t A = 0;
    uint32_t B = 0;
    uint32_t aluOpcode;
    uint32_t aluOutput;
    uint32_t readValue;

    switch (opcode) {
    case LUI:
        rf->write(immediate, rd);
        break;
    case AUIPC:
        aluOpcode = aluc->getAluOperation(opcode, 0, 0);
        aluOutput = alu->execute(pcAddr, immediate, aluOpcode);
        rf->write(aluOutput, rd);
        break;
    case JAL:
        aluOpcode = aluc->getAluOperation(opcode, 0, 0);
        aluOutput = alu->execute(pcAddr, 4, aluOpcode);
        rf->write(aluOutput, rd);
        break;
    case JALR:
        A = rf->read(rs1);
        aluOpcode = aluc->getAluOperation(opcode, 0, 0);
        aluOutput = alu->execute(pcAddr, 4, aluOpcode);
        rf->write(aluOutput, rd);
        break;
    case BTYPE:
        A = rf->read(rs1);
        B = rf->read(rs2);
        break;
    case LOAD:
        accessSize = mcu->getMemSize(funct3);
        exceptionCode = bus->read(rf->read(rs1) + immediate, accessSize,
                                  &readValue);
        if (exceptionCode != STATUS_OK) {
            // "Throw" exception
            return exceptionCode;
        }
        rf->write(readValue, rd);
        break;
    case STORE:
        accessSize = mcu->getMemSize(funct3);
        exceptionCode = bus->write(rf->read(rs1) + immediate,
                                   rf->read(rs2), accessSize);
        if (exceptionCode != STATUS_OK) {
            // "Throw" exception
            return exceptionCode;
        }
        break;
    case ITYPE:
        aluOpcode = aluc->getAluOperation(opcode, funct7, funct3);
        aluOutput = alu->execute(rf->read(rs1), immediate, aluOpcode);
        rf->write(aluOutput, rd);
        break;
    case RTYPE:
        aluOpcode = aluc->getAluOperation(opcode, funct7, funct3);
        aluOutput = alu->execute(rf->read(rs1), rf->read(rs2), aluOpcode);
        rf->write(aluOutput, rd);
        break;
    case PRIV:
        switch (funct3) {
        case 0b000: // ECALL / MRET
            switch (immediate) {
            case MRET_IMM:
                // MRET Instruction
                // Restore MIE field from MPIE
                if (csr->getMpie()) { // Set MIE if MPIE is 1
                    csr->setMie();
                }
                // Set MPIE to 1
                csr->setMpie();
                // Set MPP to privilege mode before trap
                csr->setMpp(MACHINE_MODE);
                break;
            case ECALL_IMM:
                // Throw ecall from machine mode
                return ECALL_FROM_M_MODE;
            }
            break;
        case 0b001:	// CSRRW
            // Write current value of CSR to rd
            rf->write(csr->readCsr(csrAddr), rd);
            // Store value of rs1 into CSR
            csr->writeCsr(csrAddr, rf->read(rs1));
            break;
        case 0b010:	// CSRRS
            // Write current value of CSR to rd
            rf->write(csr->readCsr(csrAddr), rd);
            // Use rs1 as a bit mask to set CSR bits
            if (rs1 != 0)
                csr->writeCsr(csrAddr, rf->read(rs1) | csr->readCsr(csrAddr));
            break;
        case 0b011:	// CSRRC
            // Write current value of CSR to rd
            rf->write(csr->readCsr(csrAddr), rd);
            // Usr rs1 as a bit mask to reset CSR bits
            if (rs1 != 0)
                csr->writeCsr(csrAddr, (!rf->read(rs1))&csr->readCsr(csrAddr));
            break;
        default:
            // Immediate CSR Instructions not yet supported
            break;
        }
        break;
    default:
        // Illegal Instruction
        return ILLEGAL_INSTRUCTION;
    }

    // Update Program Counter
    exceptionCode = nextPc->calculateNextPc(immediate, opcode, funct3, A, B,
                                            csr->getMepc());
    if (exceptionCode != STATUS_OK) {
        return exceptionCode;
    }

    pc->setPc(nextPc->getNextPc());
    
    return STATUS_OK;
}

void Mcu::debugPreExecute(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t rs1, uint32_t rs2, uint32_t rd, uint32_t immediate, uint32_t csrAddr, uint32_t curInstruction) {
    if (curInstruction == 0) {exit(1);}
    std::cout << std::hex << "Current Instruction: " << curInstruction << std::endl;
    switch (opcode) {
    case LUI: std::cout << "immediate: " << immediate << " rd: " << rd << " opcode: " << opcode << std::endl; break;
    case ITYPE: std::cout << "immediate: " << immediate << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << " opcode: " << opcode << std::endl; break;
    case RTYPE: std::cout << "rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << " opcode: " << opcode << std::endl; break;
    case LOAD: std::cout << "immediate: " << immediate << " rs1: " << rs1 << " funct3: " << funct3 << " rd: " << rd << " opcode: " << opcode << std::endl; break;
    case STORE: std::cout << "immediate: " << immediate << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " opcode: " << opcode << std::endl; break;
    case BTYPE: std::cout << "immediate: " << immediate << " rs2: " << rs2 << " rs1: " << rs1 << " funct3: " << funct3 << " opcode: " << opcode << std::endl; break;
    case JAL: std::cout << "immediate: " << immediate << " rd: " << rd << " opcode: " << opcode << std::endl; break;
    case AUIPC: std::cout << "immediate: " << immediate << " rd: " << rd << " opcode: " << opcode << std::endl; break;
    case JALR: std::cout << "immediate: " << immediate << " rd: " << rd << " rs: " << rs1 << " opcode: " << opcode << std::endl; break;
    case PRIV: // ecall/csr
        if (funct3 == 0) {
            std::cout << "ecall" << std::endl;
        }
        break;
    }
}

void Mcu::debugPostExecute(uint32_t opcode, uint32_t rd, uint32_t immediate, uint32_t rdData, uint32_t rs2Data, uint32_t rs1Data, uint32_t pcAddr) {
    switch (opcode) {
        case LUI: std::cout << "Wrote " << rdData << " to register " << rd << std::endl << std::endl; break;
        case ITYPE: std::cout << "Wrote " << rdData << " to register " << rd << std::endl << std::endl; break;
        case RTYPE: std::cout << "Wrote " << rdData << " to register " << rd << std::endl << std::endl; break;
        case LOAD: std::cout << "Wrote " << rdData << " to register " << rd << std::endl << std::endl; break;
        case STORE: std::cout << "Wrote " << rs2Data << " to address " << rs1Data + immediate << std::endl << std::endl; break;
        case BTYPE: std::cout << "Next PC: " << pcAddr << std::endl << std::endl; break;
        case JAL: std::cout << "Next PC: " << pcAddr << ". Wrote " << rdData << " to register " << rd << std::endl << std::endl; break;
        case AUIPC: std::cout << "Wrote " << pcAddr - 4 + immediate << " to register " << rd << std::endl << std::endl; break;
        case JALR: std::cout << "Next PC: " << pcAddr << ". Wrote " << rdData << " to register " << rd << std::endl << std::endl; break;
        case PRIV: std::cout << "Jump to handler at: " << pcAddr << std::endl << std::endl; break;
    }
}
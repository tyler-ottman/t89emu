#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#define BYTE                                1
#define HALFWORD                            2
#define WORD                                4

// opcodes
#define LUI                                 0b0110111
#define AUIPC                               0b0010111
#define JAL                                 0b1101111
#define JALR                                0b1100111
#define BTYPE                               0b1100011
#define LOAD                                0b0000011
#define STORE                               0b0100011 
#define ITYPE                               0b0010011
#define RTYPE                               0b0110011
#define PRIV                                0b1110011

#define ECALL_IMM                           0b000000000000
#define MRET_IMM                            0b001100000010
#define URET_IMM                            0b000000000010

#define STATUS_OK                           0xffffffff

// Interrupt codes
#define SUPERVISOR_SOFTWARE_INTERRUPT       0X80000001
#define MACHINE_SOFTWARE_INTERRUPT          0x80000003
#define SUPERVISOR_TIMER_INTERRUPT          0x80000005
#define MACHINE_TIMER_INTERRUPT             0x80000007
#define SUPERVISOR_EXTERNAL_INTERRUPT       0x80000009
#define MACHINE_EXTERNAL_INTERRUPT          0x8000000b

// Exception codes
#define INSTRUCTION_ADDRESS_MISALIGNED      0x00000000
#define INSTRUCTION_ACCESS_FAULT            0x00000001
#define ILLEGAL_INSTRUCTION                 0x00000002
#define BREAKPOINT                          0x00000003
#define LOAD_ADDRESS_MISALIGNED             0x00000004
#define LOAD_ACCESS_FAULT                   0x00000005
#define STORE_ADDRESS_MISALIGNED            0x00000006
#define STORE_ACCESS_FAULT                  0x00000007
#define ECALL_FROM_U_MODE                   0x00000008
#define ECALL_FROM_S_MODE                   0x00000009
#define ECALL_FROM_M_MODE                   0x0000000b
#define INSTRUCTION_PAGE_FAULT              0x0000000c
#define LOAD_PAGE_FAULT                     0x0000000d
#define STORE_PAGE_FAULT                    0x0000000f

#endif // ARCHITECTURE_H
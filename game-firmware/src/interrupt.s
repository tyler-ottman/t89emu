.section .text
.global _vector_table
_vector_table:
    # Interrupts
    j _reserved                             # Reserved
    j _reserved                             # Supervisor Software Interrupt
    j _reserved                             # Reserved
    j _machine_software_interrupt           # Machine Software Interrupt
    j _reserved                             # Reserved
    j _reserved                             # Supervisor Timer Interrupt
    j _reserved                             # Reserved
    j _machine_timer_interrupt              # Machine Timer Interrupt
    j _reserved                             # Reserved
    j _reserved                             # Supervisor External Interrupt
    j _reserved                             # Reserved
    j _machine_external_interrupt           # Machine External Interrupt
    j _reserved                             # Reserved
    j _reserved                             # Reserved
    j _reserved                             # Reserved
    j _reserved                             # Reserved

    # Exceptions
    j _instruction_address_misaligned       # Insturction Address Misaligned
    j _instruction_access_fault             # Instruction Access fault
    j _illegal_instruction                  # Illegal Instruction
    j _reserved                             # Breakpoint
    j _load_address_misaligned              # Load Address Misaligned
    j _load_access_fault                    # Load Access Fault
    j _store_address_misaligned             # Store/AMO address misaligned
    j _store_access_fault                   # Store/AMO access fault
    j _environment_call_u_mode              # Environment Call from U-mode
    j _reserved                             # Environment Call from S-mode
    j _reserved                             # Reserved
    j _environment_call_m_mode              # Environment Call from M-Mode

_reserved:
    mret

_machine_software_interrupt:                # Handle ECALL / Exceptions
    mret

_machine_timer_interrupt:                   # Handle Timer Interrupts
    mret

_machine_external_interrupt:                # Handle External Interrupts
    mret


_instruction_address_misaligned:
    mret

_instruction_access_fault:
    mret

_illegal_instruction:
    mret

_load_address_misaligned:
    mret

_load_access_fault:
    mret

_store_address_misaligned:
    mret

_store_access_fault:
    mret

_environment_call_u_mode:
    mret

_environment_call_m_mode:
    mret

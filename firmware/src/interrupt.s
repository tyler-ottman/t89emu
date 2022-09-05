.section .text
.global _vector_table
.global random_routine

# Do not change unless you know what you are doing
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
    j _reserved                             # Environment Call from U-mode
    j _reserved                             # Environment Call from S-mode
    j _reserved                             # Reserved
    j _environment_call_m_mode              # Environment Call from M-Mode


# This exception should never be reached
_reserved:
    jal _reserved

 # Handle ECALL / Exceptions
# Probably don't need, only 1 Hart
_machine_software_interrupt:
    mret # not tested

# Handle Timer Interrupts
_machine_timer_interrupt:                   
    mret # Write software to handle interrupt

# Handle External Interrupts
_machine_external_interrupt:
    mret # TODO

# Trap taken when branch/jump provides
# a misaligned address
_instruction_address_misaligned:
    jal _instruction_address_misaligned # Write software to handle interrupt

# What causes this exception?
_instruction_access_fault:
    jal _instruction_access_fault # TODO

# Trap taken when CPU tries to write to
# ROM or an unknown opcode is found (there might
# be other types of illegal instruction exceptions
# in the near future)
_illegal_instruction:
    jal _illegal_instruction # Write software to handle exception

# Trap taken when address being accessed from a memory 
# device is misaligned (depending on the size of the access)
_load_address_misaligned:
    jal _load_address_misaligned # Write software to handle exception
 
# Trap taken when bus module determines no memory device
# exists at the address the cpu is trying to access (possibly
# other types of load access fault exceptions in the future)
_load_access_fault:
    jal _load_access_fault # Write software to handle exception

# Trap taken when address being accessed from a memory
# device is misaligned (depending on the size of the access)
_store_address_misaligned:
    jal _store_address_misaligned # Write software to handle exception

# Trap taken when bus module determines no memory device
# exists at the address the cpu is trying to access (possibly
# other types of store access fault exceptions in future)
_store_access_fault:
    jal _store_access_fault # Write software to handle exception

# Trap taken when CPU invokes ecall instruction
_environment_call_m_mode:
    jal _environment_call_m_mode # Write software to handle exception
    mret

.section .text
.global _vector_table
.global random_routine

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

_reserved:
    mret

_machine_software_interrupt:                # Handle ECALL / Exceptions
                                            # Probably don't need, only 1 Hart
    mret # not tested

_machine_timer_interrupt:                   # Handle Timer Interrupts
    mret # tested

_machine_external_interrupt:                # Handle External Interrupts
    mret # TODO

_instruction_address_misaligned:            # Trap taken when branch/jump provides
                                            # a misaligned address
    mret # TODO

_instruction_access_fault:
    mret # TODO                             # What causes this exception?

_illegal_instruction:                       # Trap taken when CPU tries to write to
                                            # ROM or an unknown opcode is found (there might
                                            # be other types of illegal instruction exceptions
                                            # in the near future)
    mret # not tested

_load_address_misaligned:                   # Trap taken when address being accessed from a memory 
                                            # device is misaligned (depending on the size of the access)
    mret # not tested

_load_access_fault:                         # Trap taken when bus module determines no memory device
                                            # exists at the address the cpu is trying to access (possibly
                                            # other types of load access fault exceptions in the future)
    mret # not tested

_store_address_misaligned:                  # Trap taken when address being accessed from a memory
                                            # device is misaligned (depending on the size of the access)
    mret # not tested

_store_access_fault:                        # Trap taken when bus module determines no memory device
                                            # exists at the address the cpu is trying to access (possibly
                                            # other types of store access fault exceptions in future)
    mret # not tested

_environment_call_m_mode:                   # Trap taken when CPU invokes ecall instruction
    mret # tested

random_routine:
    nop
    nop
    ret

_spin_trap:
    jal _spin_trap
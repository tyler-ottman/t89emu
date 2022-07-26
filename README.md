# T89-EMU
A RISC-V Emulator built for Embedded and Operating System emulation

## Features
 * 32-bit RV32I
 * C/C++ Support
 * Interrupt Handling (M-Mode)
 * LCD Display
 * Disassembler for Debugging

## Usage
T89-EMU uses the RISC-V GNU Compiler Toolchain found <a href="https://github.com/riscv-collab/riscv-gnu-toolchain" target="_blank">here</a>. A guide outlining the build process can be found <a href="https://mindchasers.com/dev/rv-getting-started" target="_blank">here</a>. Most importantly, make sure to configure the cross compiler to properly target T89-EMU.

```console
$ cd riscv-gnu-toolchain/
$ mkdir build
$ cd build
$ ../configure --prefix=/opt/riscv32 --with-arch=rv32i --with-abi=ilp32
$ make
```
**Note**: The toolchain may take up to an hour to build

After building the toolchain

## Getting Started

The firmware directory provides a skeleton for Embedded or Operating Systems development. The code provided demonstrates how to properly interface T89-EMU's hardware (see details below detailing the hardware specifications). The emulator provides a modern graphical user interface using Dear ImGui with an OpenGL/GLFW backend. To build the emulator, execute the following commands

```console
cd build/
./build.sh
```

Once the emulator is built, navigate to the firmware directory
```console
cd ../game-firmware/
```
The Makefile provided compiles and links the source code and outputs an ELF file which targets the RISC-V T89-EMU system. After obtaining the binary, the emulator is ready to run

```console
cd ../top-level/
./rungame.sh
```

## Hardware Documentation

#### Memory Layout
Address                 | Memory Section 
---                     | --- 
0x00000000 - 0x0001FFFF | Instruction Memory
0x10000000 - 0x100FFFFF | Data Memory
0x20000000 - 0x2008FFFF | Video Memory
0x30000000 - 0x30000010 | CSR Memory

**Note**: The location of Instruction/Data Memory can be re-configured in the linker script, but the changes must also be reflected in the emulator's source code.

#### Control State Registers (CSRs)

Refer to the RISC-V privileged spec for a complete description of the control state registers. Below are details specific to T89-EMU's CSR implementation.
##### Memory-mapped CSRs
Address                 | CSR                   | Size (bytes) 
---                     | ---                   | ---
0x30000000              | mcycle                | 8
0x30000008              | mtimecmp              | 8
0x30000010              | keyboard              | 4

##### mcycle
Number of cycles since beginning of simulation

##### mtimecmp
Because the architecture is 32-bit, mtimecmp is split into 2 32-bit registers

##### keyboard
(WIP) - Buttons are mapped to a bit in the keyboard register. If a key is pressed, the corresponding bit becomes high, low otherwise

##### mstatus

Bits    | 31-13 | 12-11 | 10-8 | 7 | 6-4 | 3 | 2-0
---     | --- | --- | --- |--- |--- |--- |---
Field   | Reserved | MPP | Reserved | MPIE | Reserved | MIE | Reserved

##### mtvec
T89-EMU supports vector tables

##### mip
Bits    | 31-12 | 11 | 10-8 | 7 | 6-4 | 3 | 2-0
---     | --- | --- | --- |--- |--- |--- |---
Field   | Reserved | MEIP | Reserved | MTIP | Reserved | MSIP | Reserved

##### mie
Bits    | 31-12 | 11 | 10-8 | 7 | 6-4 | 3 | 2-0
---     | --- | --- | --- |--- |--- |--- |---
Field   | Reserved | MEIE | Reserved | MTIE | Reserved | MSIE | Reserved

## Future Ideas

 * C/C++ Decompiler
 * Read firmware directly from ELF file
 * Add User and Supervisor Protection Levels
 * Cleaner UI Support
 * Port build system to CMake
 * macOS/Windows support (soon)

## Developer Remarks
I am the only developer maintaining this project. I made the project to better understand my knowledge of computer science-related fields.
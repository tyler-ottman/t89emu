# T89-EMU
A RISC-V Emulator built for Embedded and Operating System emulation

## Features
 * RV32I_zicsr
 * C/C++ Firmware Support
 * Machine Mode Privilege Level
   * Interrupt/Syscall handling
 * Framebuffer Device (Text/Graphics Mode)
 * Core-Local Interruptor (CLINT) Device
 * Debugging Interfaces
   * Source-Level Debugger
   * Disassembled Text Viewer
   * Register/Memory Viewer

## Usage
T89-EMU uses the RISC-V GNU Compiler Toolchain found <a href="https://github.com/riscv-collab/riscv-gnu-toolchain" target="_blank">here</a>. Configure the cross compiler toolchain to target the proper base instruction set and extensions as demonstrated below. Place the toolchain directory in the same parent directory of t89emu.

```console
$ cd riscv-gnu-toolchain/
$ mkdir build
$ cd build
$ ../configure --prefix=/opt/riscv32 --with-arch=rv32im_zicsr --with-abi=ilp32
$ sudo make
```
**Note**: The toolchain may require additional dependencies and take an hour to build.

## Getting Started

The firmware directory provides a skeleton template for an Embedded/Operating systems project. The code provided demonstrates how to properly interface T89-EMU's hardware (see details below detailing the hardware specifications) and provides a modern graphical user interface using Dear ImGui. To build the emulator, execute the following commands:

```console
$ ./build.sh
```

Once the emulator is built, the firmware directory will contain an ELF file representing t89emu's firmware. The ELF file is used as input to initialize the emulator. Now, execute the following command to run the emulator:

```console
$ ./run.sh
```

A window with the application will appear. Running the sample 'Hello world' application, the window should look like:
![Alt text](./img/sample2.png "Hello World Example Pt. 2")

## Hardware Documentation

#### Memory Layout
Address                 | Memory Section 
---                     | --- 
0x20000000 - 0x2008FFFF | Video Memory
0x30000000 - 0x30000014 | CLINT Memory
0x40000000 - 0x400FFFFF | Data (RAM) Memory
0x80000000 - 0x8001FFFF | Instruction (ROM) Memory

**Note**: The location of Instruction/Data Memory can be re-configured in the linker script. Ensure memory devices do not intersect.

#### Control State Registers (CSRs)

Refer to the RISC-V privileged spec for a complete description of the control state registers. Below are details specific to memory-mapped CSRs.
##### Memory-mapped CSRs
Address                 | CSR                   | Size (bytes) 
---                     | ---                   | ---
0x30000000              | mcycle                | 8
0x30000008              | mtimecmp              | 8
0x30000010              | keyboard              | 4
0x30000014              | msip                  | 4

##### mcycle
Number of cycles since beginning of simulation

##### mtimecmp
Because the architecture is 32-bit, mtimecmp and mcycle are split into 2 32-bit registers each.

##### keyboard
(WIP)

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

#### Video Memory
The Video Memory Device divides into several sections.

Address                 | Video Segment         | Size (bytes) 
---                     | ---                   | ---
0x20000000              | Controller            | 16
0x20000010              | Text Buffer           | 1344
0x20000550              | Pixel Buffer          | 589824

##### Video Controller
Address                 | Video Segment         | Size (bytes) 
---                     | ---                   | ---
0x20000000              | Video Mode            | 1
0x20000001 - 0x2000000f | Unused                | 15
0x20000010              | Graphics/Text Buffer  | Varies

Most bytes of the video controller are unused or reserved for later use. However, the first byte defines the mode of the video controller. By default, the video mode byte initializes to 0. The video mode byte can be changed in software by setting the byte to 1 (Video Text Mode) or 2 (Video Graphics Mode)

##### Video Text Buffer

The Video Text Buffer is a character buffer located at physical memory address 0x20000010. When Video Text Mode is enabled, the characters stored in the Video Text Buffer will be displayed to the Video module. The display can print up to 21 lines of characters, each line fitting a maximum of 64 characters.

##### Video Graphics Mode

The Video Graphics Buffer is a contigious array of 32-bit pixel data located at physical memory address 0x20000550. When enabled, graphics mode displays the a 512x288 resolution image to the LCD display module. This is a work in progress.

##### 32-bit Pixel
Byte    | 3   | 2   | 1   | 0   |
---     | --- | --- | --- | --- |
Field   | A | B | G | R

## Future Ideas

 * C/C++ Source-Level Debugger :white_check_mark:
 * Read firmware directly from ELF file :white_check_mark:
 * Implement exceptions in vector table :white_check_mark:
 * Add UART/PLIC hardware support
 * Add F and M extension hardware support
 * Design a more extensive graphics mode to support Tiles/Palettes
 * Add User and Supervisor Protection Levels
 * Design and run T89's architecture on an FPGA
 * Possible extensions to project 
    - Using LLVM to create a backend to target the RISC-V architecture designed
      for T89-EMU (the scope of backend development is vast and likely warrants
      a separate project of its own)
    - Using LLVM to develop a front end for a custom programming language
    

## Developer Remarks
I am the only developer maintaining this project. I made the project to better understand my knowledge of computer science-related fields.

## Other Images
![Alt text](./img/sample1.png "Hello World Example Pt. 1")

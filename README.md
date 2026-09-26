![image](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/Embedded%20Hacking.png?raw=true)

<br>

---
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:
1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
---

<br>

## FREE Reverse Engineering Self-Study Course [HERE](https://github.com/mytechnotalent/Reverse-Engineering)

<br>

# Today's Tutorial [September 25, 2026]
## Lesson 306: Embedded Hacking Course (Chapter 24: Debugging Constants)
This chapter covers debugging constants as well as an intro to I2C as we work a 1602 LCD as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

<br>

# Embedded Hacking
A FREE comprehensive step-by-step embedded hacking course covering Embedded Software Development to Reverse Engineering.

VIDEO PROMO [HERE](https://www.youtube.com/watch?v=aD7X9sXirF8)

<br>

# FREE Book [Download](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf)

<br>

# Required Skills
Students must have a working understanding of the following items:

- **Intermediate C Programming:** In particular, you must understand how pointers reference memory addresses which we will use to perform live variable hijacking.
  - Training Link [HERE](https://www.learn-c.org)
- **Bare-Metal Embedded Systems Assembly Basics:** You must have experience with bare-metal assembly as we will be using this extensively throughout the entire course specifically with GDB and Ghidra.
  - Training Link [HERE](https://azeria-labs.com/writing-arm-assembly-part-1)
- **Computer Architecture Basics (Registers & Stack):** You need a working mental model of how a CPU/MCU uses registers and the stack to follow the boot process and function calls.
  - Training Link [HERE](https://www.geeksforgeeks.org/computer-science-fundamentals/what-is-register-memory)
  - Training Link [HERE](https://www.geeksforgeeks.org/computer-organization-architecture/memory-stack-organization-in-computer-architecture)
- **Command Line Interface (CLI) Proficiency:** You must be comfortable navigating directories and executing commands in a terminal to run the necessary OpenOCD and GDB tools.
  - Training Link [HERE](https://www.freecodecamp.org/news/command-line-commands-cli-tutorial)
- **Breadboarding Proficiency:** You must be comfortable demonstrating breadboard proficiency by efficiently prototyping circuits and integrating diverse peripherals, sensors, and microcontrollers.
  - Training Video [HERE](https://www.youtube.com/watch?v=fq6U5Y14oM4)

<br>

# Hardware

### Reference Diagrams & Schematics
- [Fritzing Project](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/hardware/EHP2.fzz)
- [Breadboard Diagram](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/hardware/EHP2_bb.png)
- [Pico 2 Pinout](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/hardware/pico-2-r4-pinout.svg)
- [Debug Probe Wiring](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/hardware/dp.png)

### Required Components & Sensors
The following hardware parts and sensors are used throughout the course experiments, projects, and live CTFs:
- [1x Full-size Breadboard (Long)](https://www.amazon.com/s?k=full+size+breadboard)
- [1x Assorted Jumper Wires (Male-to-Male, Male-to-Female, Female-to-Female)](https://www.amazon.com/s?k=breadboard+jumper+wires+assortment)
- [1x Raspberry Pi Pico 2 w/ Header](https://www.amazon.com/s?k=raspberry+pi+pico+2+with+pre-soldered+header)
- [1x Raspberry Pi Pico Debug Probe](https://www.amazon.com/s?k=raspberry+pi+debug+probe)
- [2x USB A-Male to USB Micro-B Cables (1 for Pico 2, 1 for Debug Probe)](https://www.amazon.com/s?k=micro+usb+cable+2+pack)
- [3x 5mm LEDs (1 Red, 1 Green, 1 Yellow)](https://www.amazon.com/s?k=5mm+led+kit)
- [3x 100, 220 or 330 Ohm Resistors (for LEDs)](https://www.amazon.com/s?k=resistor+assortment+kit)
- [1x Push Button (Tactile switch)](https://www.amazon.com/s?k=tactile+push+button+assortment)
- [1x 1602 LCD (with PCF8574 I2C backpack)](https://www.amazon.com/s?k=1602+lcd+i2c+module)
- [1x DHT11 Temperature & Humidity Sensor](https://www.amazon.com/s?k=dht11+temperature+and+humidity+sensor)
- [1x SG90 Servo Motor](https://www.amazon.com/s?k=sg90+micro+servo+motor)
- [1x 1000uF 25V Capacitor (for Servo power stabilization)](https://www.amazon.com/Cionyce-Capacitor-Electrolytic-CapacitorsMicrowave/dp/B0B63CCQ2N)
- [1x Infrared (IR) Receiver (VS1838B)](https://www.amazon.com/s?k=vs1838b+ir+receiver+module)
- [1x Infrared (IR) Remote Controller (NEC-compatible)](https://www.amazon.com/s?k=arduino+ir+remote+control)
- [1x u-blox NEO-6M GPS Receiver Module (with Active Antenna)](https://www.amazon.com/Navigation-Positioning-Microcontroller-Compatible-Sensitivity/dp/B084MK8BS2)
- [2x REYAX RYLR998 868/915MHz LoRa Transceiver Modules (1x Pico 2, 1x Computer)](https://www.amazon.com/dp/B099RM1XMG)
- [1x FT232RL USB to TTL Serial Adapter Module](https://www.amazon.com/FT232RL-Serial-Adapter-Module-Arduino/dp/B0FHP71BCQ)

<br><br>

# Breadboard Design
![image](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/hardware/EHP2_bb.png?raw=true)

<br>

# Datasheets & References

This repository includes the complete set of authoritative technical reference manuals and datasheets required for professional embedded engineering and reverse engineering on the Raspberry Pi Pico 2:

### [RP2350 Datasheet](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/datasheets/rp2350-datasheet.pdf)
*RP2350 Datasheet: A microcontroller by Raspberry Pi (1,380 pages)*
- **Core Architecture:** Dual Arm Cortex-M33 / Hazard3 RISC-V processors running up to 150MHz, 520kB on-chip SRAM across 10 striped banks, and 8kB one-time-programmable (OTP) storage.
- **System Memory Map (Chapter 2):** Complete physical address map covering external XIP Flash (`0x10000000`), SRAM (`0x20000000`), APB/AHB peripherals (`0x40000000`), and core-local Single-Cycle I/O (`0xd0000000`).
- **Peripheral Register Definitions:** Exhaustive register layouts, reset states, and bitfield descriptions for all 52 peripherals: `IO_BANK0` (pin multiplexing & `FUNCSEL`), `PADS_BANK0` (electrical drive strength, pulls, and Schmitt triggers), `UART0`/`UART1`, `SPI0`/`SPI1`, `I2C0`/`I2C1`, `PWM`, `DMA`, `CLOCKS`, and `WATCHDOG`.
- **Programmable I/O (PIO):** Comprehensive architectural guide for the 3 on-chip PIO blocks (`PIO0`, `PIO1`, `PIO2`) with 12 state machines for deterministic, high-speed custom hardware protocols.
- **Fast GPIO Coprocessor:** Hardware specification for single-cycle atomic GPIO output and direction control via dedicated ARM coprocessor instructions (`mcrr p0`).
- **Bootrom & Hardware Security:** Covers the internal mask ROM boot flow, secure boot signature verification, SHA-256 cryptographic accelerator, and OTP key protection.

### [Raspberry Pi Pico C/C++ SDK](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/datasheets/raspberry-pi-pico-c-sdk.pdf)
*Raspberry Pi Pico-series C/C++ SDK: Libraries and tools for development (821 pages)*
- **SDK Architecture & Build System:** CMake configuration, target library linking (`pico_stdlib`, `hardware_gpio`), compiler flags, and the UF2 binary packaging workflow.
- **Hardware Drivers (`hardware_*`):** Low-level driver API documentation:
  - `hardware_gpio`: Functions like `gpio_init()`, `gpio_set_dir()`, `gpio_put()`, `gpio_get()`, and hardware pin interrupt handling.
  - `hardware_uart`: Serial communication initialization (`uart_init()`, baud rate dividers, FIFO handling).
  - `hardware_dma`: Channel configuration, transfer sizing, and background memory pacing.
  - `hardware_clocks` & `hardware_resets`: PLL frequency configuration and peripheral unreset sequencing.
- **High-Level Runtimes (`pico_*`):** Standard I/O stream redirection (`stdio_init_all()` over UART/USB CDC), multicore core 1 launching (`pico_multicore`), sleep timers, and thread synchronization.
- **Hardware Structs (`hardware_structs`):** C struct definitions matching chip MMIO registers 1-to-1, used by reverse engineers to reconstruct decompiled peripheral accesses.

### [Arm Cortex-M33 Technical Reference Manual](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/datasheets/arm_cortex_m33_trm_100230_0100_03_en.pdf)
*Arm® Cortex®-M33 Processor Technical Reference Manual (Doc ID: 100230_0100_03_en, 168 pages)*
- **Core Microarchitecture:** 3-stage in-order pipeline, Harvard bus architecture (instruction and data buses), integer execution core, and optional IEEE 754 single/double-precision floating-point unit (FPU).
- **Nested Vectored Interrupt Controller (NVIC):** Interrupt priorities, exception vector table mapping, low-latency interrupt entry, and hardware tail-chaining mechanics.
- **Memory Protection & TrustZone:** Memory Protection Unit (MPU) supporting up to 16 configurable regions, and Security Attribution Unit (SAU) for hardware-enforced Secure vs Non-secure domain isolation.
- **CoreSight Debug & Trace:** Debug Access Port (DAP) used by OpenOCD and the Raspberry Pi Debug Probe, Flash Patch and Breakpoint unit (FPB), Data Watchpoint and Trace (DWT), and Instrumentation Trace Macrocell (ITM).
- **Coprocessor Interface:** Bus architecture connecting hardware accelerators directly to the core (utilized by the RP2350 for single-cycle GPIO and math acceleration).

### [Armv8-M Architecture Reference Manual (DDI0553B)](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/datasheets/DDI0553B_y_armv8m_arm.pdf)
*Armv8-M Architecture Reference Manual (Doc ID: DDI0553B, 2,149 pages)*
- **Authoritative Instruction Set Reference (Section C2.4, Pages 527–1454):** Complete alphabetical encyclopedia of every assembly instruction supported by the RP2350 Cortex-M33:
  - Data processing: `MOV`, `MOVS`, `MVN`, `ADD`, `SUB`, `MUL`, `SDIV`, `UDIV`, `AND`, `ORR`, `EOR`, `BIC`.
  - Memory load/store: `LDR`, `STR`, `LDRB`, `STRB`, `LDRH`, `STRH`, `LDRD`, `STRD`, `LDM`, `STM`, `PUSH`, `POP`.
  - Control flow: `B`, `BL`, `BX`, `BLX`, `CBZ`, `CBNZ`, `TBB`, `TBH`.
  - Coprocessor instructions: `MCR`, `MRC`, `MCRR`, `MRRC` (used by the RP2350 fast GPIO block).
  - Floating-point instructions: `VMOV`, `VADD`, `VSUB`, `VMUL`, `VDIV`, `VCMP`, `VMRS`, `VMSR`.
- **Machine Opcode Encodings:** Exact 16-bit Thumb and 32-bit Thumb-2 binary bit patterns for every instruction, essential for binary patching and shellcode analysis.
- **Programmer's Model:** General-purpose registers (`r0`–`r12`), Stack Pointers (`SP`/`MSP`/`PSP`), Link Register (`LR`), Program Counter (`PC`), Status Registers (`APSR`, `IPSR`, `EPSR`, `xPSR`), and Special Registers (`CONTROL`, `PRIMASK`).
- **Exception Mechanics:** Hardware state stacking upon exception entry (`r0`-`r3`, `r12`, `lr`, `pc`, `xPSR`), EXC_RETURN values, and fault analysis (HardFault, MemManage, BusFault, UsageFault).

### [Procedure Call Standard for the Arm Architecture (AAPCS32)](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/datasheets/aapcs32.pdf)
*Procedure Call Standard for the Arm® Architecture (AAPCS32 / ABI, 41 pages)*
- **Register Allocation Contract:** Defines caller and callee responsibilities across function calls:
  - Parameter passing: `r0`–`r3` pass the first four 32-bit parameters (or 64-bit pairs like `r0`-`r1` and `r2`-`r3`). Additional parameters are pushed to the stack.
  - Return values: `r0` returns 32-bit scalar values; `r0`-`r1` returns 64-bit integers and 64-bit IEEE 754 `double` floats.
  - Scratch / Caller-Saved registers: `r0`–`r3`, `r12` (`IP`), and `r14` (`LR`) can be freely overwritten by called functions.
  - Preserved / Callee-Saved registers: `r4`–`r11` must be preserved across function calls (saved via `push` in prologue and restored via `pop` in epilogue).
- **Floating-Point ABI:** Register allocation for hardware VFP registers (`s0`–`s15`, `d0`–`d7`) vs software floating-point library calls.
- **Data Alignment & Struct Packing:** Alignment rules (1, 2, 4, 8 bytes), structure member padding, and composite type memory layouts.

### [ARM Application Note 132 (advnote132)](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/datasheets/advnote132.pdf)
*ABI Advisory Note – SP must be 8-byte aligned on entry to AAPCS-conforming functions (10 pages)*
- **The 8-Byte Stack Alignment Rule:** Mandates that the Stack Pointer (`SP`) must be aligned to an 8-byte (doubleword) boundary at all public function call interfaces.
- **Why Unaligned Stacks Fail:** 64-bit memory operations (`LDRD`, `STRD`) and double-precision floating-point operations require 8-byte alignment; misaligned stacks trigger hardware alignment faults or severe memory bus performance penalties.
- **Prologue Disassembly Deconstruction:** Explains why compiler-generated assembly frequently emits `push {r4, lr}` (saving two 32-bit registers) even when `r4` is completely unused—the extra register push serves as deliberate padding to keep `SP` strictly 8-byte aligned!

<br>

# Syllabus

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Week 1 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK01/WEEK01-SLIDES.pdf)

### Week 1 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK01/WEEK01.md)

### Week 1a Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK01/WEEK01a.md)

### Chapter 2a: Understanding the ARM Stack
This chapter uses OpenOCD and GDB to step through inline Cortex-M33 stack instructions and inspect each saved register in SRAM.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK01/WEEK01a.pdf) to read the FREE pdf lesson.

### Chapter 1: hello, world
This chapter covers the basics of setting up a dev environment and basic template firmware for the Pico 2 MCU in addition to printing hello, world.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 2: Debugging hello, world
This chapter covers the debugging of our firmware for the Pico 2 MCU hello, world program.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 2
Hello, World - Debugging and Hacking Basics: Debugging and Hacking a Basic Program for the Pico 2

### Week 2 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK02/WEEK02-SLIDES.pdf)

### Week 2 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK02/WEEK02.md)

### Chapter 3: Hacking hello, world
This chapter covers the hacking of our firmware for the Pico 2 MCU hello, world program.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 3
Embedded System Analysis: Understanding the RP2350 Architecture w/ Comprehensive Firmware Analysis

### Week 3 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK03/WEEK03-SLIDES.pdf)

### Week 3 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK03/WEEK03.md)

### Ghidra Patching Tutorial [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK03/GHIDRA_PATCHING_TUTORIAL.pdf)

### CTF-01 Instructions [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0001b_ctf/CTF-01-I.md)

### CTF-01 Rubric [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0001b_ctf/CTF-01-R.md)

### CTF-01 Solution [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0001b_ctf/CTF-01-S.md)

### CTF-01 BIN [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0001b_ctf/CTF-01.bin)

### CTF-01 UF2 [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0001b_ctf/CTF-01.uf2)

### Chapter 4: Embedded System Analysis
This chapter covers a comprehensive embedded system analysis reviewing parts of the RP2350 datasheet and helpful firmware analysis tools.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 4
Variables in Embedded Systems: Debugging and Hacking Variables w/ GPIO Output Basics

### Week 4 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK04/WEEK04-SLIDES.pdf)

### Week 4 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK04/WEEK04.md)

### Week 4a Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK04/WEEK04a.md)

### Chapter 10a: Hardware-Aware Reverse Engineering with CMSIS-SVD
This chapter covers hardware-aware reverse engineering on stripped .bin firmware using CMSIS-SVD with live GDB dynamic inspection and Ghidra static analysis on the RP2350.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK04/WEEK04a.pdf) to read the FREE pdf lesson.

### Chapter 5: Intro To Variables
This chapter covers an introduction to variables as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 6: Debugging Intro To Variables
This chapter covers debugging an introduction to variables as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 7: Hacking Intro To Variables
This chapter covers hacking an introduction to variables as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 8: Uninitialized Variables
This chapter covers uninitialized variables as well as an intro to GPIO outputs as we blink an LED as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 9: Debugging Uninitialized Variables
This chapter covers debugging uninitialized variables as well as an intro to GPIO outputs as we blink an LED as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 10: Hacking Uninitialized Variables
This chapter covers hacking uninitialized variables as well as an intro to GPIO outputs as we blink an LED as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 5
Integers and Floats in Embedded Systems: Debugging and Hacking Integers and Floats w/ Intermediate GPIO Output Assembler Analysis

### Week 5 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05-SLIDES.pdf)

### Week 5 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05.md)

### Week 5a Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05a.md)

### Week 5b Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05b.md)

### Float/Hex Converter Tool [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/float_hex_converter.py)

### Chapter 19a: Hardware-Accelerated Numerics: ARM Cortex-M33 FPU vs. RP2350 Double-Precision Coprocessor
This chapter covers hardware-accelerated numerics on the RP2350, contrasting ARM Cortex-M33 single-precision FPU hardware with the custom RP2350 Double-Precision Coprocessor (DCP), with live GDB dynamic inspection, IEEE 754 reversing, and Ghidra binary patching.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05a.pdf) to read the FREE pdf lesson.

### Chapter 19b: Deep-Dive Hardware Architecture: RP2350 PADS_BANK0, IO_BANK0, SIO, and the GPIO Coprocessor (GPIOC)
This chapter covers the complete silicon architecture of RP2350 GPIO: the physical/analog pad ring (PADS_BANK0), the digital crossbar switchboard (IO_BANK0), fast single-cycle processor I/O (SIO), and the single-cycle GPIO Coprocessor (GPIOC) on ARM Cortex-M33 Coprocessor Port p0, with register-level reverse engineering, GDB hardware debugging, and Ghidra decompilation.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK05/WEEK05b.pdf) to read the FREE pdf lesson.

### Chapter 11: Integer Data Type
This chapter covers the integer data type in addition to a deeper assembler dive into GPIO outputs as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 12: Debugging Integer Data Type
This chapter covers debugging the integer data type in addition to a deeper assembler dive into GPIO outputs as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 13: Hacking Integer Data Type
This chapter covers hacking the integer data type in addition to a deeper assembler dive into GPIO outputs as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 14: Floating-Point Data Type
This chapter covers the floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 15: Debugging Floating-Point Data Type
This chapter covers debugging the floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 16: Hacking Floating-Point Data Type
This chapter covers hacking the floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 17: Double Floating-Point Data Type
This chapter covers the double floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 18: Debugging Double Floating-Point Data Type
This chapter covers debugging the double floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 19: Hacking Double Floating-Point Data Type
This chapter covers hacking the double floating-point data type as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 6
Static Variables in Embedded Systems: Debugging and Hacking Static Variables w/ GPIO Input Basics

### Week 6 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK06/WEEK06-SLIDES.pdf)

### Week 6 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK06/WEEK06.md)

### Classified Brief 0x01 [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0011a_cb/CLASSIFIED-BRIEF-0x01.md)

### Classified Brief 0x01 BIN [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0011a_cb/0x0011a_cb.bin)

### Classified Brief 0x01 UF2 [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0011a_cb/0x0011a_cb.uf2)

### Float/Hex Converter Tool [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0011a_cb/scripts/float_hex_converter.py)

### Telemetry Monitor Tool [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0011a_cb/scripts/telemetry_monitor.py)

### Chapter 20: Static Variables
This chapter covers static variables as well as an intro to GPIO inputs as we work with push buttons as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 21: Debugging Static Variables
This chapter covers debugging static variables as well as an intro to GPIO inputs as we work with push buttons as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 22: Hacking Static Variables
This chapter covers hacking static variables as well as an intro to GPIO inputs as we work with push buttons as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 7
Constants in Embedded Systems: Debugging and Hacking Constants w/ 1602 LCD I2C Basics

### Week 7 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK07/WEEK07-SLIDES.pdf)

### Week 7 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK07/WEEK07.md)

### Chapter 23: Constants
This chapter covers constants as well as an intro to I2C as we work a 1602 LCD as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 24: Debugging Constants
This chapter covers debugging constants as well as an intro to I2C as we work a 1602 LCD as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 25: Hacking Constants
This chapter covers hacking constants as well as an intro to I2C as we work a 1602 LCD as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 8
### Midterm Exam

### CTF-02 Instructions [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0017a_ctf/CTF-02-I.md)

### CTF-02 Rubric [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0017a_ctf/CTF-02-R.md)

### CTF-02 Solution [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0017a_ctf/CTF-02-S.md)

### CTF-02 BIN [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0017a_ctf/CTF-02.bin)

### CTF-02 UF2 [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/0x0017a_ctf/CTF-02.uf2)

## Week 9
Operators in Embedded Systems: Debugging and Hacking Operators w/ DHT11 Temperature & Humidity Sensor Single-Wire Protocol Basics

### Week 9 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK09/WEEK09-SLIDES.pdf)

### Week 9 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK09/WEEK09.md)

### Chapter 26: Operators
This chapter covers operators as well as an intro to single-wire protocol as we work a DHT11 temperature and humidity sensor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 27: Debugging Operators
This chapter covers debugging operators as well as an intro to single-wire protocol as we work a DHT11 temperature and humidity sensor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 28: Hacking Operators
This chapter covers hacking operators as well as an intro to single-wire protocol as we work a DHT11 temperature and humidity sensor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 10
Conditionals in Embedded Systems: Debugging and Hacking Static & Dynamic Conditionals w/ SG90 Servo Motor PWM Basics

### Week 10 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK10/WEEK10-SLIDES.pdf)

### Week 10 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK10/WEEK10.md)

### Chapter 29: Static Conditionals
This chapter covers static conditionals as well as an intro to PWM as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 30: Debugging Static Conditionals
This chapter covers debugging static conditionals as well as an intro to PWM as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 31: Hacking Static Conditionals
This chapter covers hacking static conditionals as well as an intro to PWM as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 32: Dynamic Conditionals
This chapter covers dynamic conditionals as well as additional PWM examples as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 33: Debugging Dynamic Conditionals
This chapter covers debugging dynamic conditionals as well as additional PWM examples as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 34: Hacking Dynamic Conditionals
This chapter covers hacking dynamic conditionals as well as additional PWM examples as we work a SG90 servo motor as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 11
Structures and Functions in Embedded Systems: Debugging and Hacking w/ IR Remote Control and NEC Protocol Basics

### Week 11 Slides [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK11/WEEK11-SLIDES.pdf)

### Week 11 Notebook [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/WEEK11/WEEK11.md)

### Chapter 35: Structures
This chapter covers structures as well as an intro to infrared basics as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 36: Debugging Structures
This chapter covers debugging structures as well as an intro to infrared basics as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 37: Hacking Structures
This chapter covers hacking structures as well as an intro to infrared basics as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 38: Functions, w/ Param, w/ Return
This chapter covers functions, w/ params and w/ a return value as well as additional infrared examples as we work a infrared receiver and infrared remote controller it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 39: Debugging Functions, w/ Param, w/ Return
This chapter covers debugging functions, w/ params and w/ a return value as well as additional infrared examples as we work a infrared receiver and infrared remote controller as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

### Chapter 40: Hacking Functions, w/ Param, w/ Return
This chapter covers hacking functions, w/ params and w/ a return value as it relates to embedded development on the Pico 2.

-> Click [HERE](https://github.com/mytechnotalent/Embedded-Hacking/blob/main/textbook/Embedded-Hacking.pdf) to read the FREE pdf book.

## Week 12
Unknown Firmware Debugging and Hacking

## Week 13
Final Review – Embedded Debugging and Hacking Techniques w/ Advanced Firmware Analysis Q&A

## Week 14
### Final Projects

### Final Project Option 1: The InfuSafe Pro Incident
In the aftermath of a catastrophic medical device failure, you are thrust into the role of an FDA forensic investigator facing an impossible crisis: 23 patients dead, 100 million recalled insulin pumps sitting in warehouses worldwide, and 2.3 million lives hanging in the balance all while the only evidence remaining is raw binary firmware after a rogue engineer destroyed every line of source code before fleeing to Montenegro. Armed only with GDB, Ghidra, and the reverse engineering skills honed over the first seven weeks of this course, you must excavate the truth from machine code, identify the lethal bugs spawned by an AI code generator called "OopsieGPT," and determine whether these devices can be salvaged to save millions in underserved communities or if $4.7 billion in humanitarian medical technology must be incinerated. This is not a simulation; this is triage at the intersection of embedded systems security and human survival.

### Final Project Option 2: Operation Dark Eclipse
Forty-two stories beneath frozen tundra, a shadow intelligence alliance called Dark Eyes operates centrifuges enriching weapons-grade material for a first strike against Washington, D.C. and Agent NIGHTINGALE gave her life to extract the single firmware file that now sits before you. Conventional warfare cannot reach this fortress buried beneath rock and concrete, but you can: as the architect of a precision cyber weapon in the tradition of Stuxnet, you must reverse engineer the RP2350-based centrifuge controller, craft binary patches that double the spin speed while falsifying every sensor readout to show nominal operation, and execute the sabotage that will cascade-destroy their enrichment program and set their nuclear ambitions back a decade. Every skill from the entire semester ARM assembly, Ghidra analysis, IEEE-754 floating-point manipulation, branch modification, log desynchronization converges in this final mission. Agent NIGHTINGALE's seven-year-old daughter still watches the driveway, waiting for a mother who will never return. Honor that sacrifice. Complete the mission. Do not fail.

<br>

# Supplemental Material (Beyond the Scope of the Course)

## Pico 2 IoT Projects & CTFs & Pi 4B/5 Embedded Linux C IoT Project & CTF

### Act I of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/cold-chain-monitor)

### Act I of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_cold-chain-monitor)

### Act II of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/access-gate)

### Act II of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_access-gate)

### Act III of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/pipeline-valve-controller)

### Act III of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_pipeline-valve-controller)

### Act IV of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/hvac-automation-node)

### Act IV of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_hvac-automation-node)

### Act V of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/industrial-tamper-system)

### Act V of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_industrial-tamper-system)

### Act VI of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/smart-logistics-dropbox)

### Act VI of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_smart-logistics-dropbox)

### Act VII of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/factory-andon-station)

### Act VII of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_factory-andon-station)

### Act VIII of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/datacenter-vent-controller)

### Act VIII of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_datacenter-vent-controller)

### Act IX of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/smart-parking-barrier)

### Act IX of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_smart-parking-barrier)

### Act X of OPERATION COLD IRON [HERE](https://github.com/mytechnotalent/chemical-warning-terminal)

### Act X of OPERATION COLD IRON CTF [HERE](https://github.com/mytechnotalent/CTF_chemical-warning-terminal)

### OPERATION TELESCREEN [HERE](https://github.com/mytechnotalent/telescreen)

### OPERATION TELESCREEN CTF [HERE](https://github.com/mytechnotalent/CTF_telescreen)

<br>

## Pico 2 W C MeshCore Project

### MeshCore Bare RP2350 [HERE](https://github.com/mytechnotalent/meshcore-bare-rp2350) 

<br>

## Pico 2 Rust Tutorial

### Chapter 1: What Is Embedded Rust?
This lesson will teach what embedded Rust is within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-01.md) to read the lesson and see the code.

### Chapter 2: Number Systems and Memory
This lesson will teach number systems and memory within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-02.md) to read the lesson and see the code.

### Chapter 3: Rust Essentials
This lesson will teach the Rust essentials within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-03.md) to read the lesson and see the code.

### Chapter 4: Ownership, Borrowing, and Lifetimes
This lesson will teach ownership, borrowing and lifetimes within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-04.md) to read the lesson and see the code.

### Chapter 5: Structs, Enums, and Pattern Matching
This lesson will teach structs, enums and pattern matching within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-05.md) to read the lesson and see the code.

### Chapter 6: Traits and Generics
This lesson will teach traits and generics within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-06.md) to read the lesson and see the code.

### Chapter 7: no_std and no_main
This lesson will teach no_std and no_main within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-07.md) to read the lesson and see the code.

### Chapter 8: Cargo, Targets, and the Toolchain
This lesson will teach cargo, targets and the toolchain within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-08.md) to read the lesson and see the code.

### Chapter 9: memory.x and the Linker Script
This lesson will teach the memory.x linker script within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-09.md) to read the lesson and see the code.

### Chapter 10: build.rs, Makefile, and Flashing
This lesson will teach the build.rs, Makefile and flashing process within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-10.md) to read the lesson and see the code.

### Chapter 11: Memory-Mapped I/O
This lesson will teach memory-mapped I/O within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-11.md) to read the lesson and see the code.

### Chapter 12: Real-Time and Concurrency
This lesson will teach real-time and concurrency within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-12.md) to read the lesson and see the code.

### Chapter 13: Futures and async/await
This lesson will teach futures and async/await within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-13.md) to read the lesson and see the code.

### Chapter 14: The Embassy Executor
This lesson will teach the Embassy executor within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-14.md) to read the lesson and see the code.

### Chapter 15: embassy-time
This lesson will teach embassy-time within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-15.md) to read the lesson and see the code.

### Chapter 16: The embassy-rp HAL
This lesson will teach the embassy-rp HAL within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-16.md) to read the lesson and see the code.

### Chapter 17: GPIO with embassy-rp
This lesson will teach GPIO with embassy-rp within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-17.md) to read the lesson and see the code.

### Chapter 18: Driver Architecture
This lesson will teach the driver architecture within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-18.md) to read the lesson and see the code.

### Chapter 19: config.rs — Blink Configuration
This lesson will teach the config.rs blink configuration within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-19.md) to read the lesson and see the code.

### Chapter 20: led.rs — The LED State Machine
This lesson will teach the led.rs LED state machine within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-20.md) to read the lesson and see the code.

### Chapter 21: main.rs — The Async Blink Loop
This lesson will teach the main.rs async blink loop within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-21.md) to read the lesson and see the code.

### Chapter 22: Button Hardware and Debouncing
This lesson will teach button hardware and debouncing within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-22.md) to read the lesson and see the code.

### Chapter 23: button.rs — The Button Controller
This lesson will teach the button.rs button controller within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-23.md) to read the lesson and see the code.

### Chapter 24: main.rs — The Button Polling Loop
This lesson will teach the main.rs button polling loop within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-24.md) to read the lesson and see the code.

### Chapter 25: Host Testing with cargo test
This lesson will teach host testing with cargo test within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-25.md) to read the lesson and see the code.

### Chapter 26: UART Fundamentals and the Echo Protocol
This lesson will teach UART fundamentals and the echo protocol within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-26.md) to read the lesson and see the code.

### Chapter 27: uart.rs — The Echo State Machine
This lesson will teach the uart.rs echo state machine within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-27.md) to read the lesson and see the code.

### Chapter 28: Interrupts and DMA — Interrupt-Driven UART
This lesson will teach interrupts and DMA interrupt-driven UART within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-28.md) to read the lesson and see the code.

### Chapter 29: main.rs — The UART Echo Loop
This lesson will teach the main.rs UART echo loop within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-29.md) to read the lesson and see the code.

### Chapter 30: The Complete Integration
This lesson will teach the complete integration within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-Rust-Tutorial/blob/main/CHAPTER-30.md) to read the lesson and see the code.

<br>

## Pico 2 Rust Drivers

### UART Driver [HERE](https://github.com/mytechnotalent/RP2350_Rust_UART_Driver)

### Blink Driver [HERE](https://github.com/mytechnotalent/RP2350_Rust_Blink_Driver)

### Button Driver [HERE](https://github.com/mytechnotalent/RP2350_Rust_Button_Driver)

<br>

## Pico 2 ARM Assembler Tutorial

### Chapter 1: What Is a Computer?
This lesson will teach you what is a computer with the fundamental model of computation that every computer shares with an intro to the RP2350 and ARM Cortex-M33.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-01.md) to read the lesson and see the code.

### Chapter 2: Number Systems — Binary, Hexadecimal, and Decimal
This lesson will teach the basics of the three main number systems which are decimal, binary and hexadecimal.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-02.md) to read the lesson and see the code.

### Chapter 3: Memory — Addresses, Bytes, Words, and Endianness
This lesson will teach the basics addresses, bytes, words and endianness within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-03.md) to read the lesson and see the code.

### Chapter 4: What Is a Register?
This lesson will teach the general purpose registers within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-04.md) to read the lesson and see the code.

### Chapter 5: Load-Store Architecture — How ARM Accesses Memory
This lesson will teach how ARM accesses memory with load and store architecture within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-05.md) to read the lesson and see the code.

### Chapter 6: The Fetch-Decode-Execute Cycle in Detail
This lesson will teach the fetch and decode cycle in more detail within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-06.md) to read the lesson and see the code.

### Chapter 7: ARM Cortex-M33 ISA Overview
This lesson will teach the ARM Cortex-M33 ISA overview within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-07.md) to read the lesson and see the code.

### Chapter 8: ARM Immediate and Move Instructions
This lesson will teach ARM immediate and move instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-08.md) to read the lesson and see the code.

### Chapter 9: ARM Arithmetic and Logic Instructions
This lesson will teach ARM arithmetic and logic instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-09.md) to read the lesson and see the code.

### Chapter 10: ARM Memory Access Instructions
This lesson will teach ARM memory access instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-10.md) to read the lesson and see the code.

### Chapter 11: ARM Branch Instructions
This lesson will teach ARM branch instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-11.md) to read the lesson and see the code.

### Chapter 12: ARM Calls, Returns, and the Stack Frame
This lesson will teach ARM calls, returns and the stack frame within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-12.md) to read the lesson and see the code.

### Chapter 13: Assembler Directives
This lesson will teach assembler directives within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-13.md) to read the lesson and see the code.

### Chapter 14: Labels, Symbols, and the Symbol Table
This lesson will teach labels, symbols and the symbol table within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-14.md) to read the lesson and see the code.

### Chapter 15: Sections, Memory Layout, and the Linker Script
This lesson will teach sections, memory layout, and the linker script within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-15.md) to read the lesson and see the code.

### Chapter 16: System Registers and Coprocessor Interface
This lesson will teach system registers and coprocessor interface within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-16.md) to read the lesson and see the code.

### Chapter 17: Bit Manipulation Patterns
This lesson will teach bit manipulation patterns within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-17.md) to read the lesson and see the code.

### Chapter 18: RP2350 Hardware Architecture
This lesson will teach RP2350 hardware architecture within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-18.md) to read the lesson and see the code.

### Chapter 19: The Linker Script
This lesson will teach the linker script within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-19.md) to read the lesson and see the code.

### Chapter 20: The Build System
This lesson will teach the build system within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-20.md) to read the lesson and see the code.

### Chapter 21: image_def.s — The PICOBIN Boot Block
This lesson will teach the PICOBIN boot block within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-21.md) to read the lesson and see the code.

### Chapter 22: constants.s — Memory Addresses and Constants
This lesson will teach memory addresses and constants within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-22.md) to read the lesson and see the code.

### Chapter 23: vector_table.s and stack.s — Boot Foundation
This lesson will teach the boot foundation within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-23.md) to read the lesson and see the code.

### Chapter 24: reset_handler.s — The Boot Sequence
This lesson will teach the boot sequence within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-24.md) to read the lesson and see the code.

### Chapter 25: xosc.s — Crystal Oscillator and Clock Configuration
This lesson will teach the crystal oscillator and clock configuration within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-25.md) to read the lesson and see the code.

### Chapter 26: reset.s — Releasing Peripherals from Reset
This lesson will teach releasing peripherals from reset within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-26.md) to read the lesson and see the code.

### Chapter 27: gpio.s Part 1 — GPIO_Config
This lesson will teach GPIO config within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-27.md) to read the lesson and see the code.

### Chapter 28: gpio.s Part 2, delay.s, and coprocessor.s — Output Control and Timing
This lesson will teach output control and timing within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-28.md) to read the lesson and see the code.

### Chapter 29: main.s — The Blink Loop
This lesson will teach the blink loop within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-29.md) to read the lesson and see the code.

### Chapter 30: Full Integration — From Source to Blinking LED
This lesson will teach the full source to blinking LED within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-ARM-ASM-Tutorial/blob/main/CHAPTER-30.md) to read the lesson and see the code.

<br>

## Pico 2 ARM Assembler Drivers

### UART Driver [HERE](https://github.com/mytechnotalent/RP2350_UART_Driver)

### Blink Driver [HERE](https://github.com/mytechnotalent/RP2350_Blink_Driver)

### Button Driver [HERE](https://github.com/mytechnotalent/RP2350_Button_Driver)

<br>

## Pico 2 RISC-V Assembler Tutorial

### Chapter 1: What Is a Computer?
This lesson will teach you what is a computer with the fundamental model of computation that every computer shares with an intro to the RP2350 and RISC-V.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-01.md) to read the lesson and see the code.

### Chapter 2: Number Systems — Binary, Hexadecimal, and Decimal
This lesson will teach the basics of the three main number systems which are decimal, binary and hexadecimal.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-02.md) to read the lesson and see the code.

### Chapter 3: Memory — Addresses, Bytes, Words, and Endianness
This lesson will teach the basics of addresses, bytes, words and endianness within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-03.md) to read the lesson and see the code.

### Chapter 4: What Is a Register?
This lesson will teach the general purpose registers within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-04.md) to read the lesson and see the code.

### Chapter 5: Load-Store Architecture — How RISC-V Accesses Memory
This lesson will teach how RISC-V accesses memory with load and store architecture within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-05.md) to read the lesson and see the code.

### Chapter 6: The Fetch-Decode-Execute Cycle in Detail
This lesson will teach the fetch, decode and execute cycle in detail within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-06.md) to read the lesson and see the code.

### Chapter 7: RISC-V Hazard3 ISA Overview
This lesson will teach the RISC-V Hazard3 ISA overview within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-07.md) to read the lesson and see the code.

### Chapter 8: RISC-V Immediate and Upper-Immediate Instructions
This lesson will teach RISC-V immediate and upper-immediate instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-08.md) to read the lesson and see the code.

### Chapter 9: RISC-V Arithmetic and Logic Instructions
This lesson will teach RISC-V arithmetic and logic instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-09.md) to read the lesson and see the code.

### Chapter 10: RISC-V Memory Access — Load and Store Deep Dive
This lesson will teach RISC-V memory access load and store instructions in deep detail within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-10.md) to read the lesson and see the code.

### Chapter 11: RISC-V Branch Instructions
This lesson will teach RISC-V branch instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-11.md) to read the lesson and see the code.

### Chapter 12: RISC-V Jumps, Calls, and Returns
This lesson will teach RISC-V jumps, calls and returns within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-12.md) to read the lesson and see the code.

### Chapter 13: RISC-V Pseudo-Instructions
This lesson will teach RISC-V pseudo-instructions within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-13.md) to read the lesson and see the code.

### Chapter 14: Assembler Directives
This lesson will teach assembler directives within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-14.md) to read the lesson and see the code.

### Chapter 15: Calling Convention and Stack Frames
This lesson will teach the calling convention and stack frames within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-15.md) to read the lesson and see the code.

### Chapter 16: Bitwise Operations for Hardware Programming
This lesson will teach bitwise operations for hardware programming within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-16.md) to read the lesson and see the code.

### Chapter 17: Memory-Mapped I/O
This lesson will teach memory-mapped I/O within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-17.md) to read the lesson and see the code.

### Chapter 18: The RP2350 — Architecture and Hardware
This lesson will teach the RP2350 architecture and hardware within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-18.md) to read the lesson and see the code.

### Chapter 19: The Linker Script — Placing Code in Memory
This lesson will teach the linker script and how code is placed in memory within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-19.md) to read the lesson and see the code.

### Chapter 20: The Build Pipeline — From Assembly to Flashable Binary
This lesson will teach the build pipeline from assembly to flashable binary within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-20.md) to read the lesson and see the code.

### Chapter 21: Boot Metadata — image_def.s
This lesson will teach the image_def.s boot metadata within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-21.md) to read the lesson and see the code.

### Chapter 22: The Constants File — constants.s
This lesson will teach the constants.s file within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-22.md) to read the lesson and see the code.

### Chapter 23: Stack and Vector Table — stack.s and vector_table.s
This lesson will teach the stack and vector table within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-23.md) to read the lesson and see the code.

### Chapter 24: Boot Sequence — reset_handler.s
This lesson will teach the reset_handler.s boot sequence within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-24.md) to read the lesson and see the code.

### Chapter 25: Oscillator Initialization — xosc.s
This lesson will teach the xosc.s oscillator initialization within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-25.md) to read the lesson and see the code.

### Chapter 26: Reset Controller — reset.s
This lesson will teach the reset.s reset controller within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-26.md) to read the lesson and see the code.

### Chapter 27: GPIO Configuration — gpio.s Part 1
This lesson will teach the gpio.s GPIO configuration part 1 within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-27.md) to read the lesson and see the code.

### Chapter 28: GPIO Set/Clear, Delay, and Coprocessor — gpio.s Part 2, delay.s, coprocessor.s
This lesson will teach the gpio.s GPIO set/clear, delay.s and coprocessor.s within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-28.md) to read the lesson and see the code.

### Chapter 29: Application Entry Point — main.s
This lesson will teach the main.s application entry point within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-29.md) to read the lesson and see the code.

### Chapter 30: Full Integration — Build, Flash, Wire, and Test
This lesson will teach the full build, flash, wire and test integration within the MCU.

-> Click [HERE](https://github.com/mytechnotalent/RP2350-RISCV-ASM-Tutorial/blob/main/CHAPTER-30.md) to read the lesson and see the code.

<br>

## Pico 2 RISC-V Assembler Drivers

### UART Driver [HERE](https://github.com/mytechnotalent/RP2350_UART_Driver_RISCV)

### Blink Driver [HERE](https://github.com/mytechnotalent/RP2350_Blink_Driver_RISCV)

### Button Driver [HERE](https://github.com/mytechnotalent/RP2350_Button_Driver_RISCV)

<br>

## Pico 2 Developer Projects

### LED Chase and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-01-blink)

### Traffic Light Phases and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-02-traffic-light)

### Hardware PWM Breathing and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-03-led-pwm-breathing)

### 1602 I2C LCD Text and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-04-lcd-hello)

### Live LCD Counter and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-05-lcd-live)

### SG90 Servo Sweep and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-06-servo-sweep)

### SG90 Servo Position and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-07-servo-position)

### SG90 Gauge Needle and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-08-servo-gauge)

### Tactile Button Poll and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-09-button-poll)

### Debounced Button Edges and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-10-button-debounce)

### Short, Long, and Double Press Events and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-11-button-events)

### DHT11 Temperature and Humidity Read and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-12-dht11-read)

### DHT11 Reading on the 1602 I2C LCD and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-13-dht11-lcd)

### DHT11 Comfort LEDs and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-14-dht11-leds)

### VS1838B NEC Decode and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-15-ir-decode)

### Remote Key Actions on the LEDs and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-16-ir-keys)

### Remote Keypad Digits on the LEDs and 1602 I2C LCD and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-17-ir-keypad)

### Remote Key Lamp Selection and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-18-ir-control-led)

### Remote Key Preset Angles on the SG90 Servo and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-19-ir-control-servo)

### Remote Key Status Pages on the 1602 I2C LCD and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-20-ir-lcd-status)

### Two Way Token Echo and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-21-lora-echo)

### DHT11 Telemetry and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-22-lora-telemetry)

### Remote Command LEDs and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-23-lora-command)

### ACK Timeout and Retry and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-24-lora-ack)

### Address Filter and Sender Attribution and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-25-lora-addressing)

### Signal Quality on the LCD and LEDs and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-26-lora-rssi)

### Buffered Readings and Flush on Link Recovery and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-27-lora-store-forward)

### Menu Navigation with the GP15 Button and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-28-lcd-menu-button)

### Menu Navigation with the IR Remote and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-29-lcd-menu-remote)

### Button and Remote Status Pages on the 1602 I2C LCD and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-30-status-pages)

### Hysteresis Vent Control with an IR Setpoint and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-31-thermostat)

### Latched Band Breach, Door Latch Servo, and an Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-32-cold-chain-alarm)

### Buffered Readings, Batched Flush, and an Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-33-data-logger)

### Gateway Command, Servo Position, and an Authenticated ACK [HERE](https://github.com/mytechnotalent/picokit-34-remote-actuator)

### Latching Alarm LED, Local Acknowledge, and an Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-35-annunciator-ack)

### Timed Vent and LED Actions with an Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-36-scheduler)

### Random-Light Reaction Game, Button Time, and an Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-37-reaction-timer)

### Timed Intersection Phases, a Pedestrian Request, and an Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-38-traffic-controller)

### Sealed Telemetry, Loopback Verify, and a Forged-Frame Reject [HERE](https://github.com/mytechnotalent/picokit-39-authenticated-telemetry)

### Per-Node Monotonic Sequence, Replay Window, and a Rejected Replay [HERE](https://github.com/mytechnotalent/picokit-40-anti-replay)

### Per-Device Key from a Device ID and Salt, Two Keys Proven Different [HERE](https://github.com/mytechnotalent/picokit-41-provisioning)

### BLAKE2b Hash-Chained Event Log and Recompute on Read [HERE](https://github.com/mytechnotalent/picokit-42-tamper-log)

### Debug Probe Breakpoints and SWD Variable Reads [HERE](https://github.com/mytechnotalent/picokit-43-swd-breakpoints)

### Debug Probe Data Watchpoint on a Status Variable [HERE](https://github.com/mytechnotalent/picokit-44-swd-watchpoints)

### Controlled Fault and Cortex-M33 Fault Status Registers [HERE](https://github.com/mytechnotalent/picokit-45-fault-analysis)

### ELF and UF2 Inspection of a Known Constant [HERE](https://github.com/mytechnotalent/picokit-46-binary-recon)

### Two-Node Mesh Relay and Authenticated Heartbeat [HERE](https://github.com/mytechnotalent/picokit-47-two-node-mesh)

### Sensor Node and Charted Gateway [HERE](https://github.com/mytechnotalent/picokit-48-gateway-dashboard)

### Fleet and Config Table with Multi-Node Gateway [HERE](https://github.com/mytechnotalent/picokit-49-fleet)

### Full Peripheral Capstone and Authenticated Telemetry [HERE](https://github.com/mytechnotalent/picokit-50-finale)

<br>

# License
[Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0)

# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 3
Embedded System Analysis: Understanding the RP2350 Architecture w/ Comprehensive Firmware Analysis

### 🎯 What You'll Learn This Week

By the end of this tutorial, you will be able to:
- Understand how the RP2350 boots from the on-chip bootrom
- Know what the vector table is and why it's important
- Trace the complete boot sequence from power-on to `main()`
- Understand XIP (Execute In Place) and how code runs from flash
- Read and analyze the startup assembly code (`crt0.S`)
- Use GDB to examine the boot process step by step
- Use Ghidra to statically analyze the boot sequence
- Understand the difference between Thumb mode addressing and actual addresses

### 🔄 Review from Weeks 1-2
This week builds on your GDB and Ghidra skills from previous weeks:
- **GDB Commands** (`x`, `b`, `c`, `si`, `disas`, `i r`) - We'll use all of these to trace the boot process
- **Memory Layout** (Flash at `0x10000000`, RAM at `0x20000000`) - Understanding where code and data live
- **Registers** (`r0`-`r12`, SP, LR, PC) - We'll watch how they're initialized during boot
- **Ghidra Analysis** - Decompiling and understanding assembly in a visual tool
- **Thumb Mode** - Remember addresses with LSB=1 indicate Thumb code

---

## 📚 The Code We're Analyzing

Throughout this week, we'll continue working with our `0x0001_hello-world.c` program:

```c
#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();

    while (true)
        printf("hello, world\r\n");
}
```

But this week, we're going **deeper** - we'll understand everything that happens BEFORE `main()` even runs! How does the chip know where `main()` is? How does the stack get initialized? Let's find out!

---

## 📚 Part 1: Understanding the Boot Process

### What Happens When You Power On?

When you plug in your Raspberry Pi Pico 2, a lot happens before your `main()` function runs! Think of it like waking up in the morning:

1. **First, your alarm goes off** (Power is applied to the chip)
2. **You open your eyes** (The bootrom starts running)
3. **You check your phone** (The bootrom looks for valid code in flash)
4. **You get out of bed** (The bootrom jumps to your program)
5. **You brush your teeth, get dressed** (Startup code initializes everything)
6. **Finally, you start your day** (Your `main()` function runs!)

Each of these steps has a corresponding piece of code. Let's explore them all!

### The RP2350 Boot Sequence Overview

```
┌─────────────────────────────────────────────────────────────────┐
│  STEP 1: Power On                                               │
│  - The Cortex-M33 core wakes up                                 │
│  - Execution begins at address 0x00000000 (Bootrom)             │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│  STEP 2: Bootrom Executes (32KB on-chip ROM)                    │
│  - This code is burned into the chip - can't be changed!        │
│  - It looks for valid firmware in flash memory                  │
│  - It checks for the IMAGE_DEF structure at 0x10000000          │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│  STEP 3: Boot Stage 2 (boot2)                                   │
│  - Configures the flash interface for fast reading              │
│  - Sets up XIP (Execute In Place) mode                          │
│  - Returns control to the bootrom                               │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│  STEP 4: Vector Table & Reset Handler                           │
│  - Bootrom reads the vector table at 0x10000000                 │
│  - Gets the initial stack pointer from offset 0x00              │
│  - Gets the reset handler address from offset 0x04              │
│  - Jumps to the reset handler!                                  │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│  STEP 5: C Runtime Startup (crt0.S)                             │
│  - Copies initialized data from flash to RAM                    │
│  - Zeros out the BSS section                                    │
│  - Calls runtime_init()                                         │
│  - Finally calls main()!                                        │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📚 Part 2: The Bootrom - Where It All Begins

### What is the Bootrom?

The **bootrom** is a 32KB piece of code that is permanently burned into the RP2350 chip at the factory. You cannot change it - it's "mask ROM" (Read Only Memory).

Think of the bootrom like the BIOS in your computer - it's the first thing that runs and is responsible for finding and loading your actual program.

### Key Bootrom Facts

| Property    | Value         | Description                        |
| ----------- | ------------- | ---------------------------------- |
| Size        | 32 KB         | Small but powerful                 |
| Location    | `0x00000000`  | The very first address in memory   |
| Modifiable? | **NO**        | Burned into silicon at the factory |
| Purpose     | Boot the chip | Find and load your firmware        |

### What Does the Bootrom Do?

1. **Initialize Hardware**: Sets up clocks, resets peripherals
2. **Check Boot Sources**: Looks for valid firmware in flash
3. **Validate Firmware**: Checks for magic markers (IMAGE_DEF)
4. **Configure Flash**: Sets up the XIP interface
5. **Jump to Your Code**: Reads the vector table and jumps to your reset handler

### The IMAGE_DEF Structure

The bootrom looks for a special marker in your firmware called **IMAGE_DEF**. This tells the bootrom "Hey, there's valid code here!"

Here's what it looks like in the Pico SDK:

```assembly
.section .picobin_block, "a"      // placed in flash
.word 0xffffded3                  // PICOBIN_BLOCK_MARKER_START ← ROM looks for this!
.byte 0x42                        // PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE
.byte 0x1                         // item is 1 word in size
.hword 0b0001000000100001         // SECURE mode (0x1021)
.byte 0xff                        // PICOBIN_BLOCK_ITEM_2BS_LAST
.hword 0x0001                     // item is 1 word in size
.byte 0x0                         // pad
.word 0x0                         // relative pointer to next block (0 = loop to self)
.word 0xab123579                  // PICOBIN_BLOCK_MARKER_END
```

**The magic numbers:**
- `0xffffded3` = Start marker ("I'm a valid Pico binary!")
- `0xab123579` = End marker ("End of the header block")

---

## 📚 Part 3: Understanding XIP (Execute In Place)

> 🔄 **REVIEW:** In Week 1, we learned that our code lives at `0x10000000` in flash memory. We used `x/1000i 0x10000000` to find our `main` function. Now we'll understand WHY code is at this address!

### What is XIP?

**XIP (Execute In Place)** means the processor can run code directly from flash memory without copying it to RAM first.

Think of it like reading a book:
- **Without XIP**: You photocopy every page into a notebook, then read from the notebook
- **With XIP**: You just read directly from the book!

### Why Use XIP?

| Advantage   | Explanation                                 |
| ----------- | ------------------------------------------- |
| Saves RAM   | Code stays in flash, RAM is free for data   |
| Faster Boot | No need to copy entire program to RAM first |
| Simpler     | Less memory management needed               |

### XIP Memory Address

The XIP flash region starts at address `0x10000000`. This is where your compiled code lives!

```
┌─────────────────────────────────────────────────────┐
│  Address: 0x10000000 (XIP Base)                     │
│  ┌─────────────────────────────────────────────────┐│
│  │  Vector Table (first thing here!)               ││
│  │  - Stack Pointer at offset 0x00                 ││
│  │  - Reset Handler at offset 0x04                 ││
│  │  - Other exception handlers...                  ││
│  ├─────────────────────────────────────────────────┤│
│  │  Your Code                                      ││
│  │  - Reset handler                                ││
│  │  - main() function                              ││
│  │  - Other functions                              ││
│  ├─────────────────────────────────────────────────┤│
│  │  Read-Only Data                                 ││
│  │  - Strings like "hello, world"                  ││
│  │  - Constant values                              ││
│  └─────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────┘
```

---

## 📚 Part 4: The Vector Table - The CPU's Instruction Manual

### What is the Vector Table?

The **vector table** is a list of addresses at the very beginning of your program. It tells the CPU:
1. Where to set the stack pointer
2. Where to start executing code (reset handler)
3. Where to go when errors or interrupts happen

Think of it like the table of contents in a book - it tells you where to find everything!

### Vector Table Layout

The vector table lives at `0x10000000` and looks like this:

| Offset | Address      | Content      | Description                 |
| ------ | ------------ | ------------ | --------------------------- |
| `0x00` | `0x10000000` | `0x20082000` | Initial Stack Pointer (SP)  |
| `0x04` | `0x10000004` | `0x1000015d` | Reset Handler (entry point) |
| `0x08` | `0x10000008` | `0x1000011b` | NMI Handler                 |
| `0x0C` | `0x1000000C` | `0x1000011d` | HardFault Handler           |

### Understanding Thumb Mode Addressing

**Important Concept Alert!**

Look at the reset handler address: `0x1000015d`. Notice it ends in `d` (an odd number)?

On ARM Cortex-M processors, all code runs in **Thumb mode**. The processor uses the **least significant bit (LSB)** of an address to indicate this:

| LSB        | Mode  | Meaning                                   |
| ---------- | ----- | ----------------------------------------- |
| `1` (odd)  | Thumb | "This is Thumb code"                      |
| `0` (even) | ARM   | "This is ARM code" (not used on Cortex-M) |

So `0x1000015d` means:
- The actual code is at `0x1000015c` (even address)
- The `+1` tells the processor "use Thumb mode"

**GDB vs Ghidra:**
- GDB shows `0x1000015d` (with Thumb bit)
- Ghidra shows `0x1000015c` (actual instruction address)
- Both are correct! They're just displaying it differently.

---

## 📚 Part 5: The Linker Script - Memory Mapping

### What is a Linker Script?

The **linker script** tells the compiler where to put different parts of your program in memory. It's like an architect's blueprint for memory!

### Finding the Linker Script

On Windows with the Pico SDK 2.2.0, you'll find it at:
```
C:\Users\<username>\.pico-sdk\sdk\2.2.0\src\rp2_common\pico_crt0\rp2350\memmap_default.ld
```

### Key Parts of the Linker Script

```ld
MEMORY
{
    INCLUDE "pico_flash_region.ld"
    RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 512k
    SCRATCH_X(rwx) : ORIGIN = 0x20080000, LENGTH = 4k
    SCRATCH_Y(rwx) : ORIGIN = 0x20081000, LENGTH = 4k
}
```

**What this means:**

| Region    | Start Address | Size     | Purpose               |
| --------- | ------------- | -------- | --------------------- |
| Flash     | `0x10000000`  | (varies) | Your code (XIP)       |
| RAM       | `0x20000000`  | 512 KB   | Main RAM              |
| SCRATCH_X | `0x20080000`  | 4 KB     | Core 0 scratch memory |
| SCRATCH_Y | `0x20081000`  | 4 KB     | Core 0 stack          |

### Where Does the Stack Come From?

The linker script calculates the initial stack pointer:

```ld
__StackTop = ORIGIN(SCRATCH_Y) + LENGTH(SCRATCH_Y);
```

Let's do the math:
- `ORIGIN(SCRATCH_Y)` = `0x20081000`
- `LENGTH(SCRATCH_Y)` = `0x1000` (4 KB)
- `__StackTop` = `0x20081000` + `0x1000` = **`0x20082000`**

This value (`0x20082000`) is what we see at offset `0x00` in the vector table!

---

## 📚 Part 6: Setting Up Your Environment (GDB - Dynamic Analysis)

> 🔄 **REVIEW:** This setup is identical to Weeks 1-2. If you need a refresher on OpenOCD and GDB connection, refer back to Week 1 Part 4 or Week 2 Part 5.

### Prerequisites

Before we start, make sure you have:
1. A Raspberry Pi Pico 2 board with debug probe connected
2. OpenOCD installed and configured
3. GDB (`arm-none-eabi-gdb`) installed
4. The "hello-world" binary loaded on your Pico 2
5. Access to the Pico SDK source files (for reference)

### Starting the Debug Session

**Terminal 1 - Start OpenOCD:**

```bash
openocd ^
  -s "C:\Users\flare-vm\.pico-sdk\openocd\0.12.0+dev\scripts" ^
  -f interface/cmsis-dap.cfg ^
  -f target/rp2350.cfg ^
  -c "adapter speed 5000"
```

**Terminal 2 - Start GDB:**

```bash
arm-none-eabi-gdb build/0x0001_hello-world.elf
```

**Connect to target:**

```gdb
(gdb) target remote :3333
(gdb) monitor reset halt
```

---

## 🔬 Part 7: Hands-On GDB Tutorial - Examining the Vector Table

> 🔄 **REVIEW:** We're using the same `x` (examine) command from Week 1. Remember: `x/Nx` shows N hex values, `x/Ni` shows N instructions, `x/s` shows strings.

### Step 1: Examine the Vector Table

Let's look at the first 4 entries of the vector table at `0x10000000`:

**Type this command:**

```gdb
(gdb) x/4x 0x10000000
```

**What this command means:**
- `x` = examine memory (Week 1 review!)
- `/4x` = show 4 values in hexadecimal
- `0x10000000` = the address of the vector table

**You should see:**

```
0x10000000 <__vectors>: 0x20082000  0x1000015d  0x1000011b  0x1000011d
```

### Step 2: Understanding What We See

> 🔄 **REVIEW:** In Week 1, we saw `sp = 0x20081fc8` when stopped at `main`. That's *after* some stack was used during boot. Here we see the *initial* stack pointer before any code runs!

Let's decode each value:

| Address      | Value        | Meaning                                  |
| ------------ | ------------ | ---------------------------------------- |
| `0x10000000` | `0x20082000` | Initial Stack Pointer - top of SCRATCH_Y |
| `0x10000004` | `0x1000015d` | Reset Handler + 1 (Thumb bit)            |
| `0x10000008` | `0x1000011b` | NMI Handler + 1 (Thumb bit)              |
| `0x1000000C` | `0x1000011d` | HardFault Handler + 1 (Thumb bit)        |

**Key Insight:** The stack pointer (`0x20082000`) is exactly what the linker script calculated! And all the handler addresses have their LSB set to `1` for Thumb mode.

### Step 3: Verify the Stack Pointer Calculation

Let's confirm our math by examining what's at `0x10000000`:

**Type this command:**

```gdb
(gdb) x/x 0x10000000
```

**You should see:**

```
0x10000000 <__vectors>: 0x20082000
```

This matches:
- `SCRATCH_Y` starts at `0x20081000`
- `SCRATCH_Y` is 4 KB (`0x1000` bytes)
- `0x20081000` + `0x1000` = `0x20082000` ✓

---

## 🔬 Part 8: Examining the Reset Handler

> 🔄 **REVIEW:** We used `x/5i` extensively in Weeks 1-2 to examine our `main` function. Now we'll use the same technique to examine the code that runs BEFORE `main`!

### Step 4: Disassemble the Reset Handler

The reset handler is where execution begins after the bootrom hands off control. Let's look at it:

**Type this command:**

```gdb
(gdb) x/3i 0x1000015c
```

**Note:** We use `0x1000015c` (even) not `0x1000015d` (odd) because we want to see the actual instructions!

**You should see:**

```
   0x1000015c <_reset_handler>: mov.w   r0, #3489660928 @ 0xd0000000
   0x10000160 <_reset_handler+4>:       ldr     r0, [r0, #0]
   0x10000162 <_reset_handler+6>:
    cbz r0, 0x1000016a <hold_non_core0_in_bootrom+6>
```

### Step 5: Understanding the Reset Handler

Let's break down what these first three instructions do:

**Instruction 1: `mov.w r0, #0xd0000000`**

This loads the address `0xd0000000` into register `r0`. But what's at that address?

That's the **SIO (Single-cycle I/O) base address**! The SIO block contains a special register called **CPUID** that tells us which core we're running on.

**Instruction 2: `ldr r0, [r0, #0]`**

This reads the value at address `0xd0000000` (the CPUID register) into `r0`.

| Core   | CPUID Value |
| ------ | ----------- |
| Core 0 | `0`         |
| Core 1 | `1`         |

**Instruction 3: `cbz r0, 0x1000016a`**

This is "Compare and Branch if Zero". If `r0` is `0` (meaning we're on Core 0), branch to `0x1000016a` to continue with startup. Otherwise, we're on Core 1 and need to handle that differently.

### Why Check Which Core We're On?

The RP2350 has **two cores**, but only **Core 0** should run the startup code! If both cores tried to initialize the same memory and peripherals, chaos would ensue.

So the reset handler checks:
- **Core 0?** → Continue with startup
- **Core 1?** → Go back to the bootrom and wait

---

## 🔬 Part 9: The Complete Reset Handler Flow

### Step 6: Examine More of the Reset Handler

Let's look at more instructions to see the full picture:

**Type this command:**

```gdb
(gdb) x/20i 0x1000015c
```

**You should see something like:**

```
0x1000015c <_reset_handler>: mov.w   r0, #3489660928 @ 0xd0000000
0x10000160 <_reset_handler+4>:       ldr     r0, [r0, #0]
0x10000162 <_reset_handler+6>:
cbz r0, 0x1000016a <hold_non_core0_in_bootrom+6>
0x10000164 <hold_non_core0_in_bootrom>:      mov.w   r0, #0
0x10000168 <hold_non_core0_in_bootrom+4>:
b.n 0x10000150 <_enter_vtable_in_r0>
0x1000016a <hold_non_core0_in_bootrom+6>:
add r4, pc, #52     @ (adr r4, 0x100001a0 <data_cpy_table>)
0x1000016c <hold_non_core0_in_bootrom+8>:    ldmia   r4!, {r1, r2, r3}
0x1000016e <hold_non_core0_in_bootrom+10>:   cmp     r1, #0
0x10000170 <hold_non_core0_in_bootrom+12>:
beq.n       0x10000178 <hold_non_core0_in_bootrom+20>
0x10000172 <hold_non_core0_in_bootrom+14>:
bl  0x1000019a <data_cpy>
0x10000176 <hold_non_core0_in_bootrom+18>:
b.n 0x1000016c <hold_non_core0_in_bootrom+8>
0x10000178 <hold_non_core0_in_bootrom+20>:
ldr r1, [pc, #84]   @ (0x100001d0 <data_cpy_table+48>)
0x1000017a <hold_non_core0_in_bootrom+22>:
ldr r2, [pc, #88]   @ (0x100001d4 <data_cpy_table+52>)
0x1000017c <hold_non_core0_in_bootrom+24>:   movs    r0, #0
0x1000017e <hold_non_core0_in_bootrom+26>:
b.n 0x10000182 <bss_fill_test>
0x10000180 <bss_fill_loop>:  stmia   r1!, {r0}
0x10000182 <bss_fill_test>:  cmp     r1, r2
0x10000184 <bss_fill_test+2>:        bne.n   0x10000180 <bss_fill_loop>
0x10000186 <platform_entry>:
ldr r1, [pc, #80]   @ (0x100001d8 <data_cpy_table+56>)
0x10000188 <platform_entry+2>:       blx     r1
```

### Step 7: Understanding the Startup Phases

The reset handler performs several phases:

```
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 1: Core Check (0x1000015c - 0x10000168)                  │
│  - Check CPUID to see which core we're on                       │
│  - If not Core 0, go back to bootrom                            │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 2: Data Copy (0x1000016a - 0x10000176)                   │
│  - Copy initialized variables from flash to RAM                 │
│  - Uses data_cpy_table for source/destination info              │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 3: BSS Clear (0x10000178 - 0x10000184)                   │
│  - Zero out all uninitialized global variables                  │
│  - C standard requires BSS to start at zero                     │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 4: Runtime Init & Main (0x10000186+)                     │
│  - Call runtime_init() for SDK setup                            │
│  - Call __libc_init_array() for C++ constructors                │
│  - Finally call main()!                                         │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔬 Part 10: Understanding the Data Copy Phase

### What is the Data Copy Phase?

> 🔄 **REVIEW:** In Week 2, we learned that flash is read-only and SRAM is read-write. That's why the startup code must COPY initialized variables from flash to RAM - they can't be modified in flash!

When you write C code like this:

```c
int my_counter = 42; // Initialized global variable
```

The value `42` is stored in flash memory (because flash is non-volatile). But variables need to live in RAM to be modified! So the startup code **copies** these initial values from flash to RAM.

### Step 8: Find the Data Copy Table

The data copy table contains entries that describe what to copy where. Let's examine it:

**Type this command:**

```gdb
(gdb) x/12x 0x100001a0
```

**You should see something like:**

```
0x100001a0 <data_cpy_table>:    0x10001b4c      0x20000110      0x200002ac 0x10001ce8
...
```

The data_cpy_table contains multiple entries. Each entry has three values:
1. **Source address** (in flash)
2. **Destination address** (in RAM)
3. **End address** (where to stop copying)

In the output above, we see:
- **First entry**: `0x10001b4c` (source), `0x20000110` (dest), `0x200002ac` (end)
- **Second entry starts**: `0x10001ce8` (source of next entry), ...

The table ends with an entry where the source address is `0x00000000` (which signals "no more entries").

### Step 9: Watch the Data Copy Loop

The data copy loop works like this:

```
┌─────────────────────────────────────────────┐
│  1. Load source, dest, end from table       │
│  2. If source == 0, we're done              │
│  3. Otherwise, copy word by word            │
│  4. Go back to step 1 for next entry        │
└─────────────────────────────────────────────┘
```

The actual code (starting at **`0x1000016c`** in the reset handler):

```assembly
0x1000016c <hold_non_core0_in_bootrom+8>:    ldmia   r4!, {r1, r2, r3}
0x1000016e <hold_non_core0_in_bootrom+10>:   cmp     r1, #0
0x10000170 <hold_non_core0_in_bootrom+12>:
beq.n       0x10000178 <hold_non_core0_in_bootrom+20>
0x10000172 <hold_non_core0_in_bootrom+14>:
bl  0x1000019a <data_cpy>
0x10000176 <hold_non_core0_in_bootrom+18>:
b.n 0x1000016c <hold_non_core0_in_bootrom+8>
```

> 💡 **Note:** You can see this code in **Step 6** earlier where we examined the reset handler with `x/20i 0x1000015c`.

---

## 🔬 Part 11: Understanding the BSS Clear Phase

### What is BSS?

**BSS** stands for "Block Started by Symbol" (historical name). It's the section of memory for **uninitialized global variables**.

When you write:

```c
int my_counter; // Uninitialized - will be in BSS
```

The C standard says this variable **must start at zero**. The BSS clear phase zeros out this entire region.

### Step 10: Examine the BSS Clear Loop

**Type this command:**

```gdb
(gdb) x/5i 0x10000178
```

**You should see:**

```
0x10000178 <hold_non_core0_in_bootrom+20>:
ldr r1, [pc, #84]   @ (0x100001d0 <data_cpy_table+48>)
0x1000017a <hold_non_core0_in_bootrom+22>:
ldr r2, [pc, #88]   @ (0x100001d4 <data_cpy_table+52>)
0x1000017c <hold_non_core0_in_bootrom+24>:   movs    r0, #0
0x1000017e <hold_non_core0_in_bootrom+26>:
b.n 0x10000182 <bss_fill_test>
0x10000180 <bss_fill_loop>:  stmia   r1!, {r0}
```

### Understanding the Loop

```
┌─────────────────────────────────────────────┐
│  r1 = start of BSS section                  │
│  r2 = end of BSS section                    │
│  r0 = 0                                     │
│                                             │
│  LOOP:                                      │
│    Store 0 at address r1                    │
│    Increment r1 by 4 bytes                  │
│    If r1 != r2, repeat                      │
└─────────────────────────────────────────────┘
```

---

## 🔬 Part 12: Examining Exception Handlers

### Step 11: Look at the Default Exception Handlers

What happens if an exception occurs (like a HardFault)? Let's look:

**Type this command:**

```gdb
(gdb) x/10i 0x10000110
```

**You should see:**

```
0x10000110 <isr_usagefault>: mrs     r0, IPSR
0x10000114 <isr_usagefault+4>:       subs    r0, #16
0x10000116 <unhandled_user_irq_num_in_r0>:   bkpt    0x0000
0x10000118 <isr_invalid>:    bkpt    0x0000
0x1000011a <isr_nmi>:        bkpt    0x0000
0x1000011c <isr_hardfault>:  bkpt    0x0000
0x1000011e <isr_svcall>:     bkpt    0x0000
0x10000120 <isr_pendsv>:     bkpt    0x0000
0x10000122 <isr_systick>:    bkpt    0x0000
0x10000124 <__default_isrs_end>:
            @ <UNDEFINED> instruction: 0xebf27188
```

### What is `bkpt`?

The `bkpt` instruction is a **breakpoint**. When executed, it stops the processor and triggers the debugger!

These are the **default** exception handlers - they just stop the program so you can debug. In your own code, you can override these with real handlers.

### Why So Many Handlers?

Each type of exception has its own handler:

| Handler         | Purpose                                    |
| --------------- | ------------------------------------------ |
| `isr_nmi`       | Non-Maskable Interrupt (can't be disabled) |
| `isr_hardfault` | Serious error (bad memory access, etc.)    |
| `isr_svcall`    | Supervisor Call (used by RTOSes)           |
| `isr_pendsv`    | Pendable Supervisor (also for RTOSes)      |
| `isr_systick`   | System Timer tick interrupt                |

---

## 🔬 Part 13: Finding Where Main is Called

### Step 12: Look at Platform Entry

After all the setup, the code finally calls `main()`. Let's find it:

**Type this command:**

```gdb
(gdb) x/10i 0x10000186
```

**You should see:**

```
0x10000186 <platform_entry>:
ldr r1, [pc, #80]   @ (0x100001d8 <data_cpy_table+56>)
0x10000188 <platform_entry+2>:       blx     r1
0x1000018a <platform_entry+4>:
ldr r1, [pc, #80]   @ (0x100001dc <data_cpy_table+60>)
0x1000018c <platform_entry+6>:       blx     r1
0x1000018e <platform_entry+8>:
ldr r1, [pc, #80]   @ (0x100001e0 <data_cpy_table+64>)
0x10000190 <platform_entry+10>:      blx     r1
0x10000192 <platform_entry+12>:      bkpt    0x0000
0x10000194 <platform_entry+14>:
b.n 0x10000192 <platform_entry+12>
0x10000196 <data_cpy_loop>:  ldmia   r1!, {r0}
0x10000198 <data_cpy_loop+2>:        stmia   r2!, {r0}
```

### Understanding Platform Entry

The platform entry code makes **three function calls** using `ldr` + `blx`:

1. **First call**: `runtime_init()` - SDK initialization
2. **Second call**: `main()` - YOUR CODE!
3. **Third call**: `exit()` - Called when main returns

After `main()` returns, `exit()` is called to handle cleanup. The `bkpt` instruction after `exit()` should never be reached - it's there to catch errors if `exit()` somehow returns.

### Step 13: Set a Breakpoint at Main

> 🔄 **REVIEW:** We've used `b main` and `b *ADDRESS` many times in Weeks 1-2. This is the same technique!

Let's verify we understand the boot process by setting a breakpoint at main:

**Type this command:**

```gdb
(gdb) b main
```

**You should see:**

```
Breakpoint 1 at 0x10000234: file 0x0001_hello-world.c, line 5.
```

**Now continue:**

```gdb
(gdb) c
```

**You should see:**

```
Continuing.

Breakpoint 1, main () at 0x0001_hello-world.c:5
5       stdio_init_all();
```

🎉 We've traced the entire boot process from power-on to `main()`!

---

## 🔬 Part 14: Understanding the Binary Info Header

### Step 14: Examine the Binary Info Header

Between the default ISRs and the reset handler, there's a special data structure called the **binary info header**. Let's look at it:

**Type this command:**

```gdb
(gdb) x/5x 0x10000138
```

**You should see:**

```
0x10000138 <__binary_info_header_end>:  0xffffded3      0x10210142      0x000001ff  0x00001bb0
0x10000148 <__binary_info_header_end+16>:       0xab123579
```

### Decoding the Binary Info Header

| Address      | Value        | Meaning                                   |
| ------------ | ------------ | ----------------------------------------- |
| `0x10000138` | `0xffffded3` | Start marker (PICOBIN_BLOCK_MARKER_START) |
| `0x1000013c` | `0x10212142` | Image type descriptor                     |
| `0x10000140` | `0x000001ff` | Item header/size field                    |
| `0x10000144` | `0x00001bb0` | Link to next block or data                |
| `0x10000148` | `0xab123579` | End marker (PICOBIN_BLOCK_MARKER_END)     |

**Why does GDB show this as instructions?**

GDB doesn't know this is data, not code! It tries to disassemble it as Thumb instructions, which results in nonsense. This is why you'll see things like:

```gdb
(gdb) x/i 0x10000138
```

```
0x10000138 <__binary_info_header_end>:       udf     #211    @ 0xd3
```

That's not real code - it's the magic number `0xffffded3` being misinterpreted!

---

## � Part 15: Static Analysis with Ghidra - Examining the Boot Sequence

> 🔄 **REVIEW:** In Week 1, we set up a Ghidra project and analyzed our hello-world binary. Now we'll use Ghidra to understand the boot sequence from a static analysis perspective!

### Why Use Ghidra for Boot Analysis?

While GDB is excellent for dynamic analysis (watching code execute), Ghidra excels at:
- **Seeing the big picture** - Understanding code flow without running it
- **Cross-references** - Finding all places that call a function
- **Decompilation** - Seeing C-like code even for assembly routines
- **Annotation** - Adding notes and renaming functions for clarity

### Step 15: Open Your Project in Ghidra

> 🔄 **REVIEW:** If you haven't created the project yet, refer back to Week 1 Part 5 for setup instructions.

1. Launch Ghidra and open your `0x0001_hello-world` project
2. Double-click on the `.elf` file to open it in the CodeBrowser
3. If prompted to auto-analyze, click **Yes**

### Step 16: Navigate to the Vector Table

1. In the **Navigation** menu, select **Go To...**
2. Type `0x10000000` and press Enter
3. You should see the vector table data

**What you'll see in the Listing view:**

```
                             //
                             // .text 
                             // SHT_PROGBITS  [0x10000000 - 0x100019cb]
                             // ram:10000000-ram:100019cb
                             //
             assume spsr = 0x0  (Default)
                             __vectors                                       XREF[4]:     Entry Point (*) , 
                             __flash_binary_start                                         runtime_init_install_ram_vector_
                             __VECTOR_TABLE                                               _elfProgramHeaders::00000028 (*) , 
                             __logical_binary_start                                       _elfSectionHeaders::00000034 (*)   
        10000000 00              undefine   00h
        10000001 20              ??         20h     
        10000002 08              ??         08h
        10000003 20              ??         20h     
        10000004 5d              ??         5Dh    ]                                         ?  ->  1000015d
        10000005 01              ??         01h
        10000006 00              ??         00h
        10000007 10              ??         10h
        10000008 1b              ??         1Bh                                              ?  ->  1000011b
        10000009 01              ??         01h
        1000000a 00              ??         00h
        1000000b 10              ??         10h
        1000000c 1d              ??         1Dh                                              ?  ->  1000011d
        1000000d 01              ??         01h
        1000000e 00              ??         00h
        1000000f 10              ??         10h
```

> 💡 **Notice:** Ghidra shows the vector table data as individual bytes by default. You can see it has labeled the start as `__vectors`, `__flash_binary_start`, `__VECTOR_TABLE`, and `__logical_binary_start`. The arrows (like `? -> 1000015d`) show that Ghidra recognizes these bytes as pointers to code addresses! To see the data formatted as 32-bit addresses instead of bytes, you can right-click and retype the data.

### Step 17: Navigate to the Reset Handler

1. In the Symbol Tree panel (left side), expand **Functions**
2. Find and click on `_reset_handler` (or search for it)
3. Alternatively, double-click on `_reset_handler` in the vector table listing

**What you'll see in the Decompile view (right panel):**

Ghidra will show you a decompiled version of the reset handler. While it won't be perfect C code (since this is hand-written assembly), it helps visualize the flow:

```c
void _reset_handler(void)

{
  bool bVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  
  if (_DAT_d0000000 != 0) {
    _DAT_e000ed08 = 0;
    bVar1 = (bool)isCurrentModePrivileged();
    if (bVar1) {
      setMainStackPointer(_gpio_set_function_masked64);
    }
                    /* WARNING: Could not recover jumptable at 0x1000015a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*pcRam00000004)(8,_gpio_set_function_masked64);
    return;
  }
  piVar5 = &data_cpy_table;
  uVar2 = 0;
  while( true ) {
    iVar3 = *piVar5;
    piVar6 = piVar5 + 1;
    piVar7 = piVar5 + 2;
    piVar5 = piVar5 + 3;
    if (iVar3 == 0) break;
    uVar2 = data_cpy(uVar2,iVar3,*piVar6,*piVar7);
  }
  for (puVar4 = (undefined4 *)&__TMC_END__; puVar4 != (undefined4 *)&end; puVar4 = puVar4 + 1) {
    *puVar4 = 0;
  }
  runtime_init();
  iVar3 = main();
                    /* WARNING: Subroutine does not return */
  exit(iVar3);
}
```

### Step 18: Trace the Path to Main

Let's find how the boot code eventually calls `main()`:

1. In the Symbol Tree, find the `main` function
2. Right-click on `main` and select **References → Show References to main**
3. This shows everywhere `main` is called from!

**You should see:**

| Location                  | Type | Label              |
| ------------------------- | ---- | ------------------ |
| `platform_entry+6`        | CALL | `blx r1` (to main) |

4. Double-click on the reference to jump to `platform_entry`

### Step 19: Examine Platform Entry

In Ghidra, look at `platform_entry`:

**Listing View:**
```
                             platform_entry
                             crt0.S:512 (2)
        10000186 14  49           ldr        r1,[DAT_100001d8 ]                               = 1000137Dh
                             crt0.S:513 (2)
        10000188 88  47           blx        r1=>runtime_init                                 void runtime_init(void)
                             crt0.S:514 (2)
        1000018a 14  49           ldr        r1,[DAT_100001dc ]                               = 10000235h
                             crt0.S:515 (2)
        1000018c 88  47           blx        r1=>main                                         int main(void)
                             crt0.S:516 (2)
        1000018e 14  49           ldr        r1,[DAT_100001e0 ]                               = 10001375h
                             crt0.S:517 (2)
        10000190 88  47           blx        r1=>exit                                         void exit(int status)
                             LAB_10000192                                    XREF[1]:     10000194 (j)   
                             crt0.S:521 (2)
        10000192 00  be           bkpt       0x0
                             crt0.S:522 (2)
        10000194 fd  e7           b          LAB_10000192
```

> 🎯 **Key Insight:** Ghidra's decompiler makes the boot sequence crystal clear! You can see exactly what functions are called before `main()`.

### Step 20: Create a Boot Sequence Graph

Ghidra can visualize the call flow:

1. With `_reset_handler` selected, go to **Window → Function Call Graph**
2. This shows a visual graph of all function calls from the reset handler
3. You can see the path: `_reset_handler` → `platform_entry` → `main`

### Comparing GDB and Ghidra for Boot Analysis

| Aspect | GDB (Dynamic) | Ghidra (Static) |
| ------ | ------------- | --------------- |
| **Sees runtime values** | ✅ Yes - register contents, memory | ❌ No - must infer from code |
| **Needs hardware** | ✅ Yes - Pico 2 must be connected | ❌ No - works offline |
| **Shows code flow** | Step-by-step execution | Full graph visualization |
| **Best for** | Watching what happens | Understanding structure |
| **Thumb bit handling** | Shows with +1 (0x1000015d) | Shows actual addr (0x1000015c) |

### Ghidra Tips for Boot Analysis

1. **Rename functions** - Right-click and rename unclear labels for future reference
2. **Add comments** - Press `;` to add inline comments explaining code
3. **Set data types** - Help Ghidra understand structures like the vector table
4. **Use bookmarks** - Mark important locations with **Ctrl+D**

---

## 📊 Part 16: Summary and Review

### The Complete Boot Sequence

```
┌─────────────────────────────────────────────────────────────────┐
│  1. POWER ON                                                    │
│     Cortex-M33 begins at 0x00000000 (bootrom)                   │
├─────────────────────────────────────────────────────────────────┤
│  2. BOOTROM                                                     │
│     - Initializes hardware                                      │
│     - Finds IMAGE_DEF at 0x10000000                             │
│     - Runs boot2 to configure flash                             │
├─────────────────────────────────────────────────────────────────┤
│  3. VECTOR TABLE (0x10000000)                                   │
│     - Reads SP from offset 0x00 → 0x20082000                    │
│     - Reads Reset Handler from offset 0x04 → 0x1000015d         │
├─────────────────────────────────────────────────────────────────┤
│  4. RESET HANDLER (0x1000015c)                                  │
│     - Checks CPUID (Core 0 continues, Core 1 waits)             │
│     - Copies .data from flash to RAM                            │
│     - Zeros .bss section                                        │
├─────────────────────────────────────────────────────────────────┤
│  5. PLATFORM ENTRY (0x10000186)                                 │
│     - Calls runtime_init()                                      │
│     - Calls main()                                              │
│     - Calls exit() when main returns                            │
├─────────────────────────────────────────────────────────────────┤
│  6. YOUR CODE RUNS!                                             │
│     main() at 0x10000234                                        │
└─────────────────────────────────────────────────────────────────┘
```

### Key Addresses to Remember

| Address      | What's There                             |
| ------------ | ---------------------------------------- |
| `0x00000000` | Bootrom (32KB, read-only)                |
| `0x10000000` | Vector table / XIP flash start           |
| `0x1000015c` | Reset handler (`_reset_handler`)         |
| `0x10000234` | Your `main()` function                   |
| `0x20000000` | Start of RAM                             |
| `0x20082000` | Initial stack pointer (top of SCRATCH_Y) |
| `0xd0000000` | SIO base (CPUID register)                |

### Weeks 1-2 Concepts We Applied

| Previous Concept | How We Used It This Week |
| ---------------- | ------------------------ |
| Memory Layout (Flash/RAM) | Understood why data must be copied from flash to RAM |
| GDB `x` command | Examined vector table, reset handler, and boot code |
| Breakpoints (`b`) | Set breakpoints to trace the boot sequence |
| Thumb Mode Addresses | Recognized LSB=1 means Thumb code in vector table |
| Stack Pointer | Saw how SP is initialized from the vector table |
| Ghidra Analysis | Used decompiler to understand boot flow |

### GDB Commands Reference

| Command          | What It Does                      | New/Review |
| ---------------- | --------------------------------- | ---------- |
| `x/Nx ADDRESS`   | Examine N hex values at ADDRESS   | Review |
| `x/Ni ADDRESS`   | Examine N instructions at ADDRESS | Review |
| `b main`         | Set breakpoint at main function   | Review |
| `b *ADDRESS`     | Set breakpoint at exact address   | Review |
| `si`             | Step one instruction              | Review |
| `c`              | Continue execution                | Review |
| `info registers` | Show all register values          | Review |
| `monitor reset halt` | Reset and halt the target     | Review |

### Key Concepts

| Concept          | Definition                                            |
| ---------------- | ----------------------------------------------------- |
| **Bootrom**      | 32KB factory-programmed ROM that initializes the chip |
| **Vector Table** | List of addresses for SP and exception handlers       |
| **XIP**          | Execute In Place - running code directly from flash   |
| **Thumb Mode**   | ARM's compact instruction set (LSB=1 in addresses)    |
| **BSS**          | Section for uninitialized globals (must be zeroed)    |
| **crt0.S**       | C Runtime startup assembly file                       |
| **Reset Handler**| First function called after power-on/reset            |
| **CPUID**        | Register identifying which CPU core is executing      |

### Ghidra Actions We Used

| Action | How to Access | Purpose |
| ------ | ------------- | ------- |
| Go To Address | Navigation → Go To... | Jump to specific memory address |
| Show References | Right-click → References → Show References to | Find all callers of a function |
| Function Call Graph | Window → Function Call Graph | Visualize call flow |
| Add Comment | Press `;` | Document your analysis |
| Rename Symbol | Right-click → Rename | Give meaningful names to functions |

---

## ✅ Practice Exercises

### Exercise 1: Trace a Reset

1. Set a breakpoint at the reset handler: `b *0x1000015c`
2. Type `monitor reset halt` then `c`
3. Single-step through the first 10 instructions with `si`
4. For each instruction, explain what it does

### Exercise 2: Find the Stack Size

1. The stack starts at `0x20082000`
2. The stack limit is `0x20078000` (from register assignments)
3. Calculate: How many bytes is the stack?
4. How many kilobytes is that?

### Exercise 3: Examine All Vectors

1. Use `x/16x 0x10000000` to see the first 16 vector table entries
2. For each entry, determine:
   - Is it a valid code address (starts with `0x1000...`)?
   - What handler does it point to?

### Exercise 4: Find Your Main Function

1. Use `info functions main` to find main
2. Examine 10 instructions at that address
3. Identify the first function call in main
4. What does that function do?

### Exercise 5: Trace Back from Main

1. When stopped at main, examine `$lr` (link register)
2. What address is stored there?
3. Disassemble that address - what function is it?
4. This shows you where main was called from!

### Exercise 6: Ghidra Boot Analysis

1. In Ghidra, navigate to `_reset_handler`
2. Use **Window → Function Call Graph** to visualize the call tree
3. Identify the path from `_reset_handler` to `main`
4. How many functions are called before `main` starts?
5. Add a comment in Ghidra explaining what each function does

---

## 🎓 Key Takeaways

### Building on Weeks 1-2

1. **GDB skills compound** - The `x`, `b`, `si`, and `disas` commands you learned in Weeks 1-2 are essential for understanding the boot process. Each week adds new applications for the same core skills.

2. **Memory layout is fundamental** - Understanding flash vs RAM from Week 2 explains why startup code must copy data and zero BSS.

3. **Ghidra complements GDB** - Dynamic analysis (GDB) shows what happens at runtime; static analysis (Ghidra) reveals the overall structure. Use both together!

### New Concepts This Week

4. **The boot process is deterministic** - Every RP2350 boots the same way, and understanding this helps you debug startup problems.

5. **The bootrom can't be changed** - It's burned into silicon. Security features depend on this immutability.

6. **The vector table is critical** - It tells the CPU where to start and how to handle errors.

7. **Thumb mode uses the LSB** - Address `0x1000015d` means "run Thumb code at `0x1000015c`".

8. **Startup code does essential work** - Copying data, zeroing BSS, and initializing the runtime all happen before `main()`.

9. **Only Core 0 runs startup** - Core 1 waits in the bootrom until explicitly started.

---

## � Security Implications

### How Boot Sequence Knowledge Applies to Security

Understanding the boot process is critical for both attackers and defenders. Knowledge of how the RP2350 boots reveals potential attack vectors and defense strategies.

#### Attack Scenarios

| Scenario | Attack | Boot Process Knowledge Required |
| -------- | ------ | ------------------------------- |
| **Firmware Replacement** | Replace the entire flash image with malicious firmware | Understanding IMAGE_DEF structure and how bootrom validates firmware |
| **Vector Table Hijacking** | Modify the reset handler address to point to malicious code | Knowing the vector table location at `0x10000000` |
| **Bootrom Exploitation** | Find bugs in the immutable bootrom to bypass security | Understanding bootrom behavior and sequence |
| **Debug Port Attack** | Use SWD/JTAG to dump firmware or inject code | Knowledge of how to halt and examine the boot process |
| **Startup Code Modification** | Change how data is copied or BSS is cleared | Understanding crt0 and runtime_init sequences |

#### Real-World Applications

**Industrial Control Systems:**
- An attacker with physical access could replace firmware to hide malicious behavior
- Understanding the boot sequence helps identify the earliest point where security checks can be added

**IoT Devices:**
- Compromised boot code could establish backdoors before the main application runs
- Secure boot implementations verify the vector table and reset handler integrity

**Medical Devices:**
- Boot-time attacks could modify critical safety parameters before device operation
- Understanding initialization helps implement tamper detection

### Defense Strategies

#### 1. Secure Boot Implementation

```
┌─────────────────────────────────────────────────────┐
│  SECURE BOOT FLOW                                   │
├─────────────────────────────────────────────────────┤
│  Bootrom (immutable)                                │
│    ↓                                                │
│  Verify IMAGE_DEF signature                         │
│    ↓                                                │
│  Verify boot2 signature                             │
│    ↓                                                │
│  Verify application signature                       │
│    ↓                                                │
│  If all valid: Jump to reset handler                │
│  If any invalid: Refuse to boot                     │
└─────────────────────────────────────────────────────┘
```

**Implementation:** Use cryptographic signatures to verify each boot stage before execution.

#### 2. Debug Port Protection

- **Production devices:** Permanently disable SWD/JTAG in final products
- **Debug authentication:** Require cryptographic challenge-response before allowing debug access
- **Fuses:** Blow hardware fuses to disable debug ports permanently

#### 3. Flash Protection

- **Read protection:** Enable flash read protection to prevent dumping firmware
- **Write protection:** Make critical boot sectors write-protected after initial programming
- **Encrypted storage:** Store firmware encrypted in flash

#### 4. Memory Protection Unit (MPU)

Configure the Cortex-M33's MPU to:
- Mark code regions as execute-only (no reading code as data)
- Separate privileged and unprivileged memory regions
- Prevent execution from RAM regions (defend against code injection)

#### 5. Boot-Time Integrity Checks

```c
// Early in reset handler or runtime_init
void verify_boot_integrity(void) {
    // Check vector table hasn't been modified
    uint32_t vector_table_checksum = calculate_checksum(0x10000000, VECTOR_TABLE_SIZE);
    if (vector_table_checksum != EXPECTED_CHECKSUM) {
        // Vector table tampered - refuse to boot
        secure_halt();
    }
    
    // Check critical data structures
    // Verify stack pointer is in valid range
    // etc.
}
```

#### 6. Anti-Tampering Hardware

- **Tamper detection:** Sensors that detect case opening or voltage glitching
- **Response actions:** Erase sensitive keys, refuse to boot, or alert monitoring systems
- **Secure elements:** Store cryptographic keys in separate tamper-resistant chips

### Lessons for Defenders

1. **The bootrom is your trust anchor** - Its immutability makes it the foundation of security. RP2350's secure boot features leverage this.

2. **Early is critical** - Security checks in the reset handler or runtime_init run before any application code, making them harder to bypass.

3. **Defense in depth** - Multiple layers (hardware fuses, encrypted storage, secure boot, MPU) make attacks much harder.

4. **Physical access = game over** - If an attacker can connect a debug probe, they can potentially compromise the device. Physical security matters!

5. **Know your boot sequence** - Understanding exactly what runs when helps you identify where to add security checks and what assets need protection.

### Security Research Value

For security researchers and penetration testers, boot sequence analysis helps:

- **Find vulnerabilities:** Many security bugs exist in startup code that runs before normal security checks
- **Develop exploits:** Understanding memory layout and initialization is essential for exploit development
- **Assess attack surface:** Knowing what's accessible at boot time reveals potential attack vectors
- **Build better defenses:** You can't defend what you don't understand

> **"To know your enemy, you must become your enemy."** - Sun Tzu

Understanding how an attacker would analyze and exploit the boot sequence is essential for building robust defenses.

---

## �📖 Glossary

### New Terms This Week

| Term              | Definition                                                              |
| ----------------- | ----------------------------------------------------------------------- |
| **Bootrom**       | Factory-programmed ROM containing first-stage bootloader                |
| **BSS**           | Block Started by Symbol - section for uninitialized global variables    |
| **CPUID**         | Register that identifies which CPU core is executing                    |
| **crt0**          | C Runtime Zero - the startup code that runs before main                 |
| **IMAGE_DEF**     | Structure that marks valid firmware for the bootrom                     |
| **Linker Script** | File that defines memory layout for the compiled program                |
| **Reset Handler** | First function called after reset/power-on                              |
| **Thumb Mode**    | Compact instruction encoding used by Cortex-M                           |
| **Vector Table**  | Array of addresses for stack pointer and exception handlers             |
| **VTOR**          | Vector Table Offset Register - tells CPU where to find the vector table |
| **XIP**           | Execute In Place - running code directly from flash memory              |

### Review Terms from Weeks 1-2

| Term | Definition | How We Used It |
| ---- | ---------- | -------------- |
| **Breakpoint** | Marker that pauses program execution | Set at reset handler and main |
| **Register** | Fast storage inside the processor | Watched SP, LR, PC during boot |
| **Stack Pointer** | Register pointing to top of stack | Saw initial value in vector table |
| **Flash Memory** | Read-only storage for code | Contains vector table and boot code |
| **SRAM** | Read-write memory for data | Where stack and variables live |

---

## 📚 Additional Resources

### RP2350 Datasheet

For more details on the boot process, see Chapter 5 of the RP2350 Datasheet:
https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf

### Pico SDK Source Code

The startup code lives in:
- `crt0.S` - Main startup assembly
- `memmap_default.ld` - Default linker script
- `boot2_generic_03h.S` - Second stage bootloader

### Bootrom Source

The bootrom source is available at:
https://github.com/raspberrypi/pico-bootrom-rp2350

---

**Remember:** Understanding the boot process is fundamental to embedded systems work. Whether you're debugging a system that won't start, reverse engineering firmware, or building secure boot chains, this knowledge is essential!

Happy exploring! 🔍

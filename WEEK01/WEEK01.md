# Week 1: Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

## 🎯 What You'll Learn This Week

By the end of this week, you will be able to: 
- Understand what a microcontroller is and how it works
- Know the basic registers of the ARM Cortex-M33 processor
- Understand memory layout (Flash vs RAM) and why it matters
- Understand how the stack works in embedded systems
- Set up and connect GDB to your Pico 2 for debugging
- Use Ghidra for static analysis of your binary
- Read basic ARM assembly instructions and understand what they do

---

## 📚 Part 1: Understanding the Basics

### What is a Microcontroller? 

Think of a microcontroller as a tiny computer on a single chip. Just like your laptop has a processor, memory, and storage, a microcontroller has all of these packed into one small chip. The **RP2350** is the microcontroller chip that powers the **Raspberry Pi Pico 2**. 

### What is the ARM Cortex-M33?

The RP2350 has two "brains" inside it - we call these **cores**.  One brain uses ARM Cortex-M33 instructions, and the other can use RISC-V instructions. In this course, we'll focus on the **ARM Cortex-M33** core because it's more commonly used in the industry.

### What is Reverse Engineering?

Reverse engineering is like being a detective for code. Instead of writing code and compiling it, we take compiled code (the 1s and 0s that the computer actually runs) and figure out what it does.  This is useful for:
- Understanding how things work
- Finding bugs or security issues
- Learning how software interacts with hardware

---

## 📚 Part 2: Understanding Processor Registers

### What is a Register? 

A **register** is like a tiny, super-fast storage box inside the processor. The processor uses registers to hold numbers while it's doing calculations. Think of them like the short-term memory your brain uses when doing math in your head.

### The ARM Cortex-M33 Registers

The ARM Cortex-M33 has several important registers:

| Register     | Also Called          | Purpose                                     |
| ------------ | -------------------- | ------------------------------------------- |
| `r0` - `r12` | General Purpose      | Store numbers, pass data between functions  |
| `r13`        | SP (Stack Pointer)   | Keeps track of where we are in the stack    |
| `r14`        | LR (Link Register)   | Remembers where to go back after a function |
| `r15`        | PC (Program Counter) | Points to the next instruction to run       |

##### General Purpose Registers (`r0` - `r12`)

These 13 registers are your "scratch paper." When the processor needs to add two numbers, subtract, or do any calculation, it uses these registers to hold the values.

**Example:** If you want to add 5 + 3:
1. Put 5 in `r0`
2. Put 3 in `r1`
3. Add them and store the result (8) in `r2`

##### The Stack Pointer (`r13` / SP)

The **stack** is a special area of memory that works like a stack of plates: 
- When you add something, you put it on top (called a **PUSH**)
- When you remove something, you take it from the top (called a **POP**)

The Stack Pointer always points to the top of this stack.  On ARM systems, the stack **grows downward** in memory.  This means when you push something onto the stack, the address number gets smaller! 

```
Higher Memory Address (0x20082000)
┌──────────────────┐
│                  │  ← Stack starts here (empty)
├──────────────────┤
│   Pushed Item 1  │  ← SP points here after 1 push
├──────────────────┤
│   Pushed Item 2  │  ← SP points here after 2 pushes
└──────────────────┘
Lower Memory Address (0x20081FF8)
```

##### The Link Register (`r14` / LR)

When you call a function, the processor needs to remember where to come back to.  The Link Register stores this "return address."

**Example:** 
```
main() calls print_hello()
   ↓
LR = address right after the call in main()
   ↓
print_hello() runs
   ↓
print_hello() finishes, looks at LR
   ↓
Jumps back to main() at the address stored in LR
```

##### The Program Counter (`r15` / PC)

The Program Counter always points to the **next instruction** the processor will execute. It's like your finger following along as you read a book - it always points to where you are. 

---

## 📚 Part 3: Understanding Memory Layout

### XIP - Execute In Place

The RP2350 uses something called **XIP (Execute In Place)**. This means the processor can run code directly from the flash memory (where your program is stored) without copying it to RAM first. 

**Key Memory Address:** `0x10000000`

This is where your program code starts in flash memory. Remember this address - we'll use it a lot!

### Memory Map Overview

```
┌─────────────────────────────────────┐
│  Flash Memory (XIP)                 │
│  Starts at:  0x10000000             │
│  Contains: Your program code        │
├─────────────────────────────────────┤
│  RAM                                │
│  Starts at: 0x20000000              │
│  Contains: Stack, Heap, Variables   │
└─────────────────────────────────────┘
```

### Stack vs Heap

| Stack                                    | Heap                               |
| ---------------------------------------- | ---------------------------------- |
| Automatic memory management              | Manual memory management           |
| Fast                                     | Slower                             |
| Limited size                             | More flexible size                 |
| Used for function calls, local variables | Used for dynamic memory allocation |
| Grows downward                           | Grows upward                       |

---

## 📚 Part 3.5: Reviewing Our Hello World Code

Before we start debugging, let's understand the code we'll be working with. Here's our `0x0001_hello-world.c` program:

```c
#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();

    while (true)
        printf("hello, world\r\n");
}
```

### Breaking Down the Code

##### The Includes

```c
#include <stdio.h>
#include "pico/stdlib.h"
```

- **`<stdio.h>`** - This is the standard input/output library. It gives us access to the `printf()` function that lets us print text.
- **`"pico/stdlib.h"`** - This is the Pico SDK's standard library. It provides essential functions for working with the Raspberry Pi Pico hardware.

##### The Main Function

```c
int main(void) {
```

Every C program starts running from the `main()` function. The `void` means it takes no arguments, and `int` means it returns an integer (though our program never actually returns).

##### Initializing Standard I/O

```c
stdio_init_all();
```

This function initializes all the standard I/O (input/output) for the Pico. It sets up:
- **USB CDC** (so you can see output when connected to a computer via USB)
- **UART** (serial communication pins)

Without this line, `printf()` wouldn't have anywhere to send its output!

##### The Infinite Loop

```c
while (true)
    printf("hello, world\r\n");
```

- **`while (true)`** - This creates an infinite loop. The program will keep running forever (or until you reset/power off the Pico).
- **`printf("hello, world\r\n")`** - This prints the text "hello, world" followed by a carriage return (`\r`) and newline (`\n`).

> 💡 **Why `\r\n` instead of just `\n`?**
> 
> In embedded systems, we often use both carriage return (`\r`) and newline (`\n`) together. The `\r` moves the cursor back to the beginning of the line, and `\n` moves to the next line. This ensures proper display across different terminal programs.

### What Happens When This Runs?

1. **Power on** - The Pico boots up and starts executing code from flash memory
2. **`stdio_init_all()`** - Sets up USB and/or UART for communication
3. **Infinite loop begins** - The program enters the `while(true)` loop
4. **Print forever** - "hello, world" is sent over and over as fast as possible

### Why This Code is Perfect for Learning

This simple program is ideal for reverse engineering practice because:
- It has a clear, recognizable function call (`printf`)
- It has an infinite loop we can observe
- It's small enough to understand completely
- It demonstrates real hardware interaction (USB/UART output)

When we debug this code, we'll be able to see how the C code translates to ARM assembly instructions!

### Compiling and Flashing to the Pico 2

Now that we understand the code, let's get it running on our hardware:

##### Step 1: Compile the Code

In VS Code, look for the **Compile** button in the status bar at the bottom of the window. This is provided by the Raspberry Pi Pico extension. Click it to compile your project.

The extension will run CMake and build your code, creating a `.uf2` file that can be loaded onto the Pico 2.

##### Step 2: Put the Pico 2 in Flash Loading Mode

To flash new code to your Pico 2, you need to put it into **BOOTSEL mode**:

1. **Press and hold** the right-most button on your breadboard (the BOOTSEL button)
2. **While holding BOOTSEL**, press the white **Reset** button
3. **Release the Reset button** first
4. **Then release the BOOTSEL button**

When done correctly, your Pico 2 will appear as a USB mass storage device (like a flash drive) on your computer. This means it's ready to receive new firmware!

> 💡 **Tip:** You'll see a drive called "RP2350" appear in your file explorer when the Pico 2 is in flash loading mode.

##### Step 3: Flash and Run

Back in VS Code, click the **Run** button in the status bar. The extension will:
1. Copy the compiled `.uf2` file to the Pico 2
2. The Pico 2 will automatically reboot and start running your code

Once flashed, your Pico 2 will immediately start executing the hello-world program, printing "hello, world" continuously when we open PuTTY!

---

## 📚 Part 4: Dynamic Analysis with GDB

### Prerequisites

Before we start, make sure you have: 
1. A Raspberry Pi Pico 2 board
2. GDB (GNU Debugger) installed
3. OpenOCD or another debug probe connection
4. The sample "hello-world" binary loaded on your Pico 2

### Connecting to Your Pico 2 with OpenOCD

Open a terminal and start OpenOCD:

```powershell
openocd ^
  -s "C:\Users\flare-vm\.pico-sdk\openocd\0.12.0+dev\scripts" ^
  -f interface/cmsis-dap.cfg ^
  -f target/rp2350.cfg ^
  -c "adapter speed 5000"
```

### Connecting to Your Pico 2 with GDB

Open another terminal and start GDB with your binary:

```powershell
arm-none-eabi-gdb build\0x0001_hello-world.elf
```

Connect to your target: 

```powershell
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
```

### Basic GDB Commands: Your First Steps

Now that we're connected, let's learn three essential GDB commands that you'll use constantly in embedded reverse engineering.

##### Setting a Breakpoint with `b main`

A **breakpoint** tells the debugger to pause execution when it reaches a specific point. Let's set one at our `main` function:

```gdb
(gdb) b main
Breakpoint 1 at 0x10000234: file ../0x0001_hello-world.c, line 5.
```

**What this tells us:**
- GDB found our `main` function
- It's located at address `0x10000234` in flash memory
- The source file and line number are shown (because we have debug symbols)

Now let's run to that breakpoint:

```gdb
(gdb) c
Continuing.

Breakpoint 1, main () at ../0x0001_hello-world.c:5
5       stdio_init_all();
```

The program has stopped right at the beginning of `main`!

##### Disassembling with `disas`

The `disas` (disassemble) command shows us the assembly instructions for the current function:

```gdb
(gdb) disas
Dump of assembler code for function main:
=> 0x10000234 <+0>:     push    {r3, lr}
   0x10000236 <+2>:     bl      0x1000156c <stdio_init_all>
   0x1000023a <+6>:     ldr     r0, [pc, #8]    @ (0x10000244 <main+16>)
   0x1000023c <+8>:     bl      0x100015fc <__wrap_puts>
   0x10000240 <+12>:    b.n     0x1000023a <main+6>
   0x10000242 <+14>:    nop
   0x10000244 <+16>:    adds    r4, r1, r7
   0x10000246 <+18>:    asrs    r0, r0, #32
End of assembler dump.
```

**Understanding the output:**
- The `=>` arrow shows where we're currently stopped
- Each line shows: `address <offset>: instruction operands`
- We can see the calls to `stdio_init_all` and `__wrap_puts` (printf was optimized to puts)
- The `b.n 0x1000023a` at the end is our infinite loop - it jumps back to reload the string!

##### Viewing Registers with `i r`

The `i r` (info registers) command shows the current state of all CPU registers:

```gdb
(gdb) i r
r0             0x0                 0
r1             0x10000235          268436021
r2             0x80808080          -2139062144
r3             0xe000ed08          -536810232
r4             0x100001d0          268435920
r5             0x88526891          -2007865199
r6             0x4f54710           83183376
r7             0x400e0014          1074659348
r8             0x43280035          1126694965
r9             0x0                 0
r10            0x10000000          268435456
r11            0x62707361          1651536737
r12            0xed07f600          -318245376
sp             0x20082000          0x20082000
lr             0x1000018f          268435855
pc             0x10000234          0x10000234 <main>
xpsr           0x69000000          1761607680
```

**Key registers to watch:**
| Register | Value        | Meaning                                         |
| -------- | ------------ | ----------------------------------------------- |
| `pc`     | `0x10000234` | Program Counter - we're at the start of `main`  |
| `sp`     | `0x20081fc8` | Stack Pointer - top of our stack in RAM         |
| `lr`     | `0x100002d5` | Link Register - where we return after `main`    |
| `r0-r3`  | Various      | Will hold function arguments and return values  |

> 💡 **Tip:** You can also use `i r pc sp lr` to show only specific registers you care about.

### Quick Reference: Essential GDB Commands

| Command               | Short Form | What It Does                         |
| --------------------- | ---------- | ------------------------------------ |
| `break main`          | `b main`   | Set a breakpoint at main             |
| `continue`            | `c`        | Continue execution until breakpoint  |
| `disassemble`         | `disas`    | Show assembly for current function   |
| `info registers`      | `i r`      | Show all register values             |
| `stepi`               | `si`       | Execute one assembly instruction     |
| `nexti`               | `ni`       | Execute one instruction (skip calls) |
| `x/10i $pc`           |            | Examine 10 instructions at PC        |
| `monitor reset halt`  |            | Reset the target and halt            |

---

> 💡 **What's Next?** In Week 2, we'll put these GDB commands to work with hands-on debugging exercises! We'll step through code, examine the stack, watch registers change, and ultimately use these skills to modify a running program. The commands you learned here are the foundation for everything that follows.

---

## 🔬 Part 5: Static Analysis with Ghidra

### Setting Up Your First Ghidra Project

Before we dive into GDB debugging, let's set up Ghidra to analyze our hello-world binary. Ghidra is a powerful reverse engineering tool that will help us visualize the disassembly and decompiled code.

##### Step 1: Create a New Project

1. Launch Ghidra
2. A window will appear - select **File → New Project**
3. Choose **Non-Shared Project** and click **Next**
4. Enter the Project Name: `0x0001_hello-world`
5. Click **Finish**

##### Step 2: Import the Binary

1. Open your file explorer and navigate to the `Embedded-Hacking` folder
2. **Drag and drop** the `0x0001_hello-world.elf` file into the folder panel within the Ghidra application

##### Step 3: Understand the Import Dialog

In the small window that appears, you will see the file identified as an **ELF** (Executable and Linkable Format). 

> 💡 **What is an ELF file?**
> 
> ELF stands for **Executable and Linkable Format**. This format includes **symbols** - human-readable names for functions and variables. These symbols make reverse engineering much easier because you can see function names like `main` and `printf` instead of just memory addresses.
> 
> In future weeks, we will work with **stripped binaries** (`.bin` files) that do not contain these symbols. This is more realistic for real-world reverse engineering scenarios where symbols have been removed to make analysis harder.

3. Click **Ok** to import the file
4. **Double-click** on the file within the project window to open it in the CodeBrowser

##### Step 4: Auto-Analyze the Binary

When prompted, click **Yes** to auto-analyze the binary. Accept the default analysis options and click **Analyze**.

Ghidra will now process the binary, identifying functions, strings, and cross-references. This may take a moment.

### Reviewing the Main Function in Ghidra

Once analysis is complete, let's find our `main` function:

1. In the **Symbol Tree** panel on the left, expand **Functions**
2. Look for `main` in the list (you can also use **Search → For Address or Label** and type "main")
3. Click on `main` to navigate to it

##### What You'll See

Ghidra shows you two views of the code:

**Listing View (Center Panel)** - The disassembled ARM assembly:
```
                             *************************************************************
                             *                           FUNCTION                          
                             *************************************************************
                             int  main (void )
                               assume LRset = 0x0
                               assume TMode = 0x1
             int               r0:4           <RETURN>
                             main                                            XREF[3]:     Entry Point (*) , 
                                                                                          _reset_handler:1000018c (c) , 
                                                                                          .debug_frame::00000018 (*)   
                             0x0001_hello-world.c:4 (2)
                             0x0001_hello-world.c:5 (2)
        10000234 08  b5           push       {r3,lr}
                             0x0001_hello-world.c:5 (4)
        10000236 01  f0  99  f9    bl         stdio_init_all                                   _Bool stdio_init_all(void)
                             LAB_1000023a                                    XREF[1]:     10000240 (j)   
                             0x0001_hello-world.c:7 (6)
                             0x0001_hello-world.c:8 (6)
        1000023a 02  48           ldr        r0=>__EH_FRAME_BEGIN__ ,[DAT_10000244 ]           = "hello, world\r"
                                                                                             = 100019CCh
        1000023c 01  f0  de  f9    bl         __wrap_puts                                      int __wrap_puts(char * s)
                             0x0001_hello-world.c:7 (8)
        10000240 fb  e7           b          LAB_1000023a
        10000242 00              ??         00h
        10000243 bf              ??         BFh
                             DAT_10000244                                    XREF[1]:     main:1000023a (R)   
        10000244 cc  19  00  10    undefine   100019CCh                                        ?  ->  100019cc

```

**Decompile View (Right Panel)** - The reconstructed C code:
```c
int main(void) {
    stdio_init_all();
    do {
        __wrap_puts("hello, world");
    } while (true);
}
```

> 🎯 **Notice how Ghidra reconstructed our original C code!** The decompiler recognized the infinite loop and the `puts` call (the compiler optimized `printf` to `puts` since we're just printing a simple string).

##### Why We Start with .elf Files

We're using the `.elf` file because it contains symbols that help us learn:
- Function names are visible (`main`, `stdio_init_all`, `puts`)
- Variable names may be preserved
- The structure of the code is easier to understand

In future weeks, we'll work with `.bin` files that have been stripped of symbols. This will teach you how to identify functions and understand code when you don't have these helpful hints!

---

## 📊 Part 6: Summary and Review

### What We Learned

1. **Registers**: The ARM Cortex-M33 has 13 general-purpose registers (`r0`-`r12`), plus special registers for the stack pointer (`r13`/SP), link register (`r14`/LR), and program counter (`r15`/PC).

2. **The Stack**: 
   - Grows downward in memory
   - PUSH adds items (SP decreases)
   - POP removes items (SP increases)
   - Used to save return addresses and register values

3. **Memory Layout**:
   - Code lives in flash memory starting at `0x10000000`
   - Stack lives in RAM around `0x20080000`

4. **GDB Basics**: We learned the essential commands for connecting to hardware and examining code:

| Command               | What It Does                           |
| --------------------- | -------------------------------------- |
| `target remote :3333` | Connect to OpenOCD debug server        |
| `monitor reset halt`  | Reset and halt the processor           |
| `b main`              | Set breakpoint at main function        |
| `c`                   | Continue running until breakpoint      |
| `disas`               | Disassemble current function           |
| `i r`                 | Show all register values               |

5. **Ghidra Static Analysis**: We set up a Ghidra project and analyzed our binary:
   - Imported the ELF file with symbols
   - Found the `main` function
   - Saw the decompiled C code
   - Understood how assembly maps to C

6. **Little-Endian**: The RP2350 stores multi-byte values with the least significant byte at the lowest address, making them appear "backwards" when viewed as a single value.

### The Program Flow

```
┌─────────────────────────────────────────────────────┐
│  1. push {r3, lr}                                   │
│     Save registers to stack                         │
├─────────────────────────────────────────────────────┤
│  2. bl stdio_init_all                               │
│     Initialize standard I/O                         │
├─────────────────────────────────────────────────────┤
│  3. ldr r0, [pc, #8]      ────────────────┐         │
│     Load address of "hello, world" into r0│         │
├─────────────────────────────────────────────────────┤
│  4. bl __wrap_puts                        │         │
│     Print the string                      │         │
├─────────────────────────────────────────────────────┤
│  5. b.n (back to step 3)  ────────────────┘         │
│     Infinite loop!                                  │
└─────────────────────────────────────────────────────┘
```

---

## ✅ Practice Exercises

Try these on your own to reinforce what you learned:

### Exercise 1: Explore in Ghidra
1. Open your `0x0001_hello-world` project in Ghidra
2. Find the `stdio_init_all` function in the Symbol Tree
3. Look at its decompiled code - can you understand what it's setting up?

### Exercise 2: Find Strings in Ghidra
1. In Ghidra, go to **Window → Defined Strings**
2. Look for `"hello, world"` - what address is it at?
3. Double-click to navigate to it in the listing

### Exercise 3: Cross-References
1. In Ghidra, navigate to the `main` function
2. Find the `ldr r0, [DAT_...]` instruction that loads the string
3. Right-click on `DAT_10000244` and select **References → Show References to**
4. This shows you where this data is used!

### Exercise 4: Connect GDB (Preparation for Week 2)
1. Start OpenOCD and connect GDB as shown in Part 4
2. Set a breakpoint at main: `b main`
3. Continue: `c`
4. Use `disas` to see the assembly
5. Use `i r` to see register values

> 💡 **Note:** The detailed hands-on GDB debugging (stepping through code, watching the stack, examining memory) will be covered in Week 2!

---

## 🎓 Key Takeaways

1. **Reverse engineering combines static and dynamic analysis** - we look at the code (static with Ghidra) and run it to see what happens (dynamic with GDB).

2. **The stack is fundamental** - understanding how push/pop work is essential for following function calls.

3. **GDB and Ghidra work together** - Ghidra helps you understand the big picture, GDB lets you watch it happen live.

4. **Assembly isn't scary** - each instruction does one simple thing. Put them together and you understand the whole program!

5. **Everything is just numbers** - whether it's code, data, or addresses, it's all stored as numbers in memory.

---

## 📖 Glossary

| Term                | Definition                                                |
| ------------------- | --------------------------------------------------------- |
| **Assembly**        | Human-readable representation of machine code             |
| **Breakpoint**      | A marker that tells the debugger to pause execution       |
| **GDB**             | GNU Debugger - a tool for examining running programs      |
| **Hex/Hexadecimal** | Base-16 number system (0-9, A-F)                          |
| **Little-Endian**   | Storing the least significant byte at the lowest address  |
| **Microcontroller** | A small computer on a single chip                         |
| **Program Counter** | Register that points to the next instruction              |
| **Register**        | Fast storage inside the processor                         |
| **Stack**           | Memory region for temporary storage during function calls |
| **Stack Pointer**   | Register that points to the top of the stack              |
| **XIP**             | Execute In Place - running code directly from flash       |

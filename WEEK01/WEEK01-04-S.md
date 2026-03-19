# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Non-Credit Practice Exercise 4 Solution: Connect GDB & Basic Exploration

#### Answers

##### Question 1: GDB Connection
- **Was GDB able to connect to OpenOCD?** Yes, via `target extended-remote localhost:3333`
- **Did program stop at breakpoint?** Yes, at `Breakpoint 1, main () at ../0x0001_hello-world.c:4`

##### Question 2: Memory Address of main
- **Address of main's first instruction:** `0x10000234`
- **Flash or RAM?** **Flash memory** - the address starts with `0x10000...` (XIP region starting at `0x10000000`)

##### Question 3: Stack Pointer Value
- **SP value at main:** `0x20082000`
- **Flash or RAM?** **RAM** - the address starts with `0x20000...` (SRAM starts at `0x20000000`)

##### Question 4: First Instruction
- **First instruction in main:** `push {r3, lr}`
- **What does it do?** Saves register `r3` and the Link Register (`lr`) onto the stack. This preserves the return address so `main()` can call other functions (like `stdio_init_all()` and `__wrap_puts`) and they can properly use `lr` themselves.

##### Question 5: Comparison to Ghidra
**Yes, they match.** The GDB disassembly output is identical to what Ghidra shows in the Listing View. Both static analysis (Ghidra) and dynamic analysis (GDB) reveal the same instructions.

##### Register Values at Breakpoint

| Register | Value | Description |
|----------|-------|-------------|
| **pc** | `0x10000234` | Program Counter - at start of main (Flash) |
| **sp** | `0x20082000` | Stack Pointer - top of stack (RAM) |
| **lr** | `0x1000018f` | Link Register - return address after main |
| **r0** | `0x0` | General Purpose - will hold function arguments |
| **r1** | `0x10000235` | General Purpose |
| **r2** | `0x80808080` | General Purpose |
| **r3** | `0xe000ed08` | General Purpose |

##### Full Disassembly of main

```
0x10000234 <+0>:     push    {r3, lr}           # Save registers to stack
0x10000236 <+2>:     bl      0x1000156c <stdio_init_all>  # Initialize I/O
0x1000023a <+6>:     ldr     r0, [pc, #8]       # Load string pointer
0x1000023c <+8>:     bl      0x100015fc <__wrap_puts>     # Print string
0x10000240 <+12>:    b.n     0x1000023a <main+6>          # Infinite loop
0x10000242 <+14>:    nop
0x10000244 <+16>:    (data: pointer to string)
```

##### GDB Connection Sequence

```
Terminal 1: openocd -s "..." -f interface/cmsis-dap.cfg -f target/rp2350.cfg
Terminal 2: arm-none-eabi-gdb build/0x0001_hello-world.elf
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) b main
(gdb) c
(gdb) disassemble main
(gdb) i r
```

#### Reflection Answers

1. **Why does the stack pointer start at `0x20082000`?**
   The initial stack pointer value comes from the first entry in the vector table at `0x10000000`. The linker script sets `__StackTop` to `0x20082000`, which is the top of the SCRATCH_Y region in SRAM. The stack grows downward from this address.

2. **Why does `push {r3, lr}` save `r3` even though it doesn't seem to be used?**
   ARM requires 8-byte stack alignment. Pushing `lr` alone would only move SP by 4 bytes. Including `r3` ensures the stack remains 8-byte aligned, which is required by the ARM Architecture Procedure Call Standard (AAPCS).

3. **How does the infinite loop work?**
   The instruction at `0x10000240` is `b.n 0x1000023a` - an unconditional branch back to `main+6`, which reloads the string pointer and calls `__wrap_puts` again. The function never returns.

# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Non-Credit Practice Exercise 4 Solution: Connect GDB & Basic Exploration

#### Answers

##### Step 1-2 Verification

- **Was GDB able to connect to OpenOCD?** Yes, via `target extended-remote localhost:3333`.
- **Did the program stop at the `main` breakpoint?** Yes, at `Breakpoint 1, main () at ../0x0001_hello-world.c:4`.

##### Step 3: Answer Exactly

- **What is the address of `main`'s first instruction, and is it Flash or RAM?**
   - `0x10000234`, and it is in **Flash** (`0x100...` XIP region).
- **What is the `sp` value at `main`, and is it Flash or RAM?**
   - `x/s $sp`, the value is, `0x20082000`, and it is in **RAM** (`0x200...` SRAM region).
- **What is the first instruction in `main`, and what does it do?**
   - `push {r3, lr}`; it saves `r3` and `lr` on the stack and keeps 8-byte stack alignment for ABI-compliant calls.
- **Does GDB match what Ghidra shows?**
   - Yes. The disassembly and flow match the Ghidra listing.

##### Step 4: Capture Register Values (`pc`, `sp`, `lr`, `r0-r3`, by using `x/x $XX`)

| Register | Value | Description |
|----------|-------|-------------|
| **pc** | `0x10000234` | Program Counter - at start of main (Flash) |
| **sp** | `0x20082000` | Stack Pointer - top of stack (RAM) |
| **lr** | `0x1000018f` | Link Register - return address after main |
| **r0** | `0x0` | General Purpose - will hold function arguments |
| **r1** | `0x10000235` | General Purpose |
| **r2** | `0x80808080` | General Purpose |
| **r3** | `0xe000ed08` | General Purpose |

##### Reference Disassembly (for verification)

```
0x10000234 <+0>:     push    {r3, lr}                     # Save registers to stack
0x10000236 <+2>:     bl      0x1000156c <stdio_init_all>  # Initialize I/O
0x1000023a <+6>:     ldr     r0, [pc, #8]                 # Load string pointer
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
(gdb) disas main
(gdb) i r
```

#### Step 5: Reflection Answers

1. **Why does the stack pointer start at `0x20082000`?**
   The initial stack pointer value comes from the first entry in the vector table at `0x10000000`. The linker script sets `__StackTop` to `0x20082000`, which is the top of the SCRATCH_Y region in SRAM. The stack grows downward from this address.

2. **Why does `push {r3, lr}` save `r3` even though it doesn't seem to be used?**
   ARM requires 8-byte stack alignment. Pushing `lr` alone would only move SP by 4 bytes. Including `r3` ensures the stack remains 8-byte aligned, which is required by the ARM Architecture Procedure Call Standard (AAPCS).

3. **How does the infinite loop work?**
   The instruction at `0x10000240` is `b.n 0x1000023a` - an unconditional branch back to `main+6`, which reloads the string pointer and calls `__wrap_puts` again. The function never returns.

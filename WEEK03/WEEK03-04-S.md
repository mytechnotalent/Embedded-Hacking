# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 3
Embedded System Analysis: Understanding the RP2350 Architecture w/ Comprehensive Firmware Analysis

### Non-Credit Practice Exercise 4 Solution: Find Your Main Function and Trace Back

#### Answers

##### Main Function Location

```gdb
(gdb) info functions main
0x10000234  int main(void);
```

`main()` is at address **0x10000234**.

##### Disassembly of main()

```gdb
(gdb) x/10i 0x10000234
0x10000234 <main>:          push   {r7, lr}
0x10000236 <main+2>:        sub    sp, #8
0x10000238 <main+4>:        add    r7, sp, #0
0x1000023a <main+6>:        bl     0x100012c4 <stdio_init_all>
0x1000023e <main+10>:       movw   r0, #404  @ 0x194
0x10000242 <main+14>:       movt   r0, #4096 @ 0x1000
0x10000246 <main+18>:       bl     0x1000023c <__wrap_puts>
0x1000024a <main+22>:       b.n    0x1000023e <main+10>
```

##### First Function Call

The first function call is at offset +6:
```
0x1000023a <main+6>: bl 0x100012c4 <stdio_init_all>
```

`stdio_init_all()` initializes all standard I/O systems (USB CDC, UART) so `printf()` and `puts()` can output to the serial console.

##### Link Register (Caller Identification)

```gdb
(gdb) b main
(gdb) c
(gdb) info registers lr
lr             0x1000018b       268435851
```

LR = **0x1000018b** (Thumb address), actual return address = **0x1000018a**.

##### Caller Disassembly

```gdb
(gdb) x/10i 0x10000186
0x10000186 <platform_entry>:      ldr    r1, [pc, #80]
0x10000188 <platform_entry+2>:    blx    r1               → calls runtime_init()
0x1000018a <platform_entry+4>:    ldr    r1, [pc, #80]    → LR points here (return from main)
0x1000018c <platform_entry+6>:    blx    r1               → THIS called main()
0x1000018e <platform_entry+8>:    ldr    r1, [pc, #80]
0x10000190 <platform_entry+10>:   blx    r1               → calls exit()
0x10000192 <platform_entry+12>:   bkpt   0x0000
```

##### Complete Boot Chain

```
Power On
  → Bootrom (0x00000000)
    → Vector Table (0x10000000)
      → _reset_handler (0x1000015c)
        → Data Copy & BSS Clear
          → platform_entry (0x10000186)
            → runtime_init() (first blx)
            → main() (second blx) ← 0x10000234
              → stdio_init_all() (first call in main)
              → puts() loop (infinite)
```

#### Reflection Answers

1. **Why does the link register point 4 bytes after the `blx` instruction that called main?**
   The LR stores the **return address**—the instruction to execute after `main()` returns. The `blx r1` instruction at `0x10000188` (which calls `runtime_init`) is 2 bytes, and the `blx r1` at `0x1000018c` (which calls `main`) is also 2 bytes. The LR is set to the instruction immediately following the `blx` that called `main()`, which is `0x1000018a` (the `ldr` after `runtime_init`'s call). Actually, `platform_entry+4` at `0x1000018a` is where execution resumes, and the actual `blx` that calls main is at `+6` (`0x1000018c`), so LR = `0x1000018e` + Thumb bit. The key point: LR always points to the next instruction after the branch, so the caller can resume where it left off.

2. **What would happen if `main()` tried to return (instead of looping forever)?**
   Execution would return to `platform_entry` at the address stored in LR. Looking at the disassembly, `platform_entry` would proceed to execute the third `blx r1` at offset +10, which calls `exit()`. The `exit()` function would perform cleanup and ultimately halt the processor. After `exit()`, there's a `bkpt` instruction as a safety net. In practice on bare-metal embedded systems, returning from `main()` is generally avoided because there's no OS to return to.

3. **How can you tell from the disassembly that main contains an infinite loop?**
   The instruction at `0x1000024a` is `b.n 0x1000023e <main+10>`, which is an **unconditional branch** back to the `movw r0, #404` instruction that loads the string address. This is a `b` (branch) with no condition code, meaning it always jumps backward. There is no path that reaches the end of the function or a `pop {r7, pc}` (return). The `push {r7, lr}` at the start saves the return address but it's never restored—the function loops forever.

4. **Why is `stdio_init_all()` called before the printf loop?**
   `stdio_init_all()` configures the hardware interfaces (USB CDC and/or UART) that `printf()`/`puts()` uses for output. Without this initialization, the serial output drivers are not set up—writes to stdout would go nowhere or cause a fault. It must be called exactly once before any I/O operation. Calling it inside the loop would repeatedly reinitialize the hardware, wasting time and potentially disrupting active connections.

# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 2
Hello, World - Debugging and Hacking Basics: Debugging and Hacking a Basic Program for the Pico 2

### Non-Credit Practice Exercise 2 Solution: Use a Different SRAM Address

#### Answers

##### Attack Summary
Write the payload to `0x20001000` instead of `0x20000000` to demonstrate that multiple safe SRAM locations can be used for injection.

##### GDB Commands

```gdb
(gdb) b *0x1000023c
(gdb) c
(gdb) set {char[14]} 0x20001000 = {'h','a','c','k','e','d','!','!','!','\r','\0'}
(gdb) set $r0 = 0x20001000
(gdb) c
```

##### Verification
```gdb
(gdb) x/s 0x20001000                   # Shows "hacked!!!\r"
```

#### Reflection Answers

1. **How can you ensure `0x20001000` does not collide with stack usage?**
   The stack pointer was observed at `0x20082000` (top of stack) and grows downward. Since `0x20001000` is far below the stack region, there is substantial separation. Use `info registers sp` in GDB to verify the current stack pointer is well above your injection address.

2. **What symptoms would indicate you overwrote an active stack frame?**
   The program would crash when attempting to return from a function. Symptoms include: unexpected address exceptions, invalid memory access faults, or the program jumping to random addresses. The Link Register return path gets corrupted.

3. **How would you pick a safe SRAM offset in a larger program with dynamic allocations?**
   Start from the bottom of SRAM (`0x20000000`) for small static payloads, working upward. Check the linker script to understand memory regions. In this simple program with no heap allocations, both `0x20000000` and `0x20001000` are safe. In larger programs, examine the `.bss` and `.data` section boundaries.

# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 3
Embedded System Analysis: Understanding the RP2350 Architecture w/ Comprehensive Firmware Analysis

### Non-Credit Practice Exercise 3 Solution: Examine All Vectors

#### Answers

##### Vector Table Dump

```gdb
(gdb) x/16x 0x10000000
0x10000000 <__vectors>:     0x20082000  0x1000015d  0x1000011b  0x1000011d
0x10000010 <__vectors+16>:  0x1000011f  0x10000121  0x10000123  0x00000000
0x10000020 <__vectors+32>:  0x00000000  0x00000000  0x00000000  0x10000125
0x10000030 <__vectors+48>:  0x00000000  0x00000000  0x10000127  0x10000129
```

##### Complete Vector Table Map

| Offset | Vector # | Raw Value    | Address Type  | Actual Addr  | Handler Name     |
|--------|----------|-------------|---------------|-------------|------------------|
| 0x00   | —        | 0x20082000  | Stack Pointer | N/A          | __StackTop       |
| 0x04   | 1        | 0x1000015d  | Code (Thumb)  | 0x1000015c  | _reset_handler   |
| 0x08   | 2        | 0x1000011b  | Code (Thumb)  | 0x1000011a  | isr_nmi          |
| 0x0C   | 3        | 0x1000011d  | Code (Thumb)  | 0x1000011c  | isr_hardfault    |
| 0x10   | 4        | 0x1000011f  | Code (Thumb)  | 0x1000011e  | isr_memmanage    |
| 0x14   | 5        | 0x10000121  | Code (Thumb)  | 0x10000120  | isr_busfault     |
| 0x18   | 6        | 0x10000123  | Code (Thumb)  | 0x10000122  | isr_usagefault   |
| 0x1C   | 7        | 0x00000000  | Reserved      | N/A          | (Reserved)       |
| 0x20   | 8        | 0x00000000  | Reserved      | N/A          | (Reserved)       |
| 0x24   | 9        | 0x00000000  | Reserved      | N/A          | (Reserved)       |
| 0x28   | 10       | 0x00000000  | Reserved      | N/A          | (Reserved)       |
| 0x2C   | 11       | 0x10000125  | Code (Thumb)  | 0x10000124  | isr_svcall       |
| 0x30   | 12       | 0x00000000  | Reserved      | N/A          | (Reserved)       |
| 0x34   | 13       | 0x00000000  | Reserved      | N/A          | (Reserved)       |
| 0x38   | 14       | 0x10000127  | Code (Thumb)  | 0x10000126  | isr_pendsv       |
| 0x3C   | 15       | 0x10000129  | Code (Thumb)  | 0x10000128  | isr_systick      |

##### Handler Verification

```gdb
(gdb) info symbol 0x1000015c
_reset_handler in section .text
(gdb) info symbol 0x1000011a
isr_nmi in section .text
(gdb) x/3i 0x1000011a
0x1000011a <isr_nmi>:       bkpt  0x0000
0x1000011c <isr_hardfault>: bkpt  0x0000
0x1000011e <isr_svcall>:    bkpt  0x0000
```

Most default handlers are single `bkpt` instructions—they halt the processor if triggered unexpectedly.

##### Summary Statistics

| Category             | Count |
|---------------------|-------|
| Stack Pointer entry | 1     |
| Valid code entries  | 10    |
| Reserved (0x0)      | 5     |
| **Total entries**   | **16** |

#### Reflection Answers

1. **Why do all the code addresses end in odd numbers (LSB = 1)?**
   ARM Cortex-M processors exclusively execute in Thumb mode. The least significant bit (LSB) of vector table entries indicates the instruction set: LSB = 1 means Thumb mode. Since Cortex-M33 only supports Thumb/Thumb-2, all code addresses must have bit 0 set to 1. The actual instruction address is the value with bit 0 cleared (e.g., `0x1000015d` → instruction at `0x1000015c`). This is a hardware requirement—loading an even address into the PC on Cortex-M would cause a HardFault.

2. **What happens if an exception occurs for a reserved/null vector entry?**
   If the processor attempts to invoke a handler through a vector entry containing `0x00000000`, it tries to fetch instructions from address `0x00000000` (in the bootrom region). This would either execute bootrom code unexpectedly or trigger a HardFault because the address lacks the Thumb bit (LSB = 0). In practice, a HardFault would occur, and if the HardFault handler itself is invalid, the processor enters a lockup state.

3. **Why do most exception handlers just contain `bkpt` instructions?**
   The Pico SDK provides these as **default weak handlers**. They are intentionally minimal—a `bkpt` (breakpoint) instruction halts the processor immediately so a debugger can catch the event. This is a safety mechanism: rather than letting an unhandled exception corrupt state or silently continue, the default handlers stop execution so the developer can diagnose the problem. Application code can override any handler by defining a function with the matching name (the weak linkage gets replaced).

4. **How would you replace a default handler with your own custom handler?**
   Define a C function with the exact handler name (e.g., `void isr_hardfault(void)`) in your application code. Because the SDK declares these handlers as `__attribute__((weak))`, the linker will use your strong definition instead of the default `bkpt` stub. The new function's address (with Thumb bit set) will automatically appear in the vector table at the correct offset. No linker script modification is needed.

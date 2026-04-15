# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 3
Embedded System Analysis: Understanding the RP2350 Architecture w/ Comprehensive Firmware Analysis

### Non-Credit Practice Exercise 1 Solution: Trace a Reset

#### Answers

##### Instruction Trace (First 10 Instructions from 0x1000015c)

| # | Address      | Instruction                            | What It Does                                         | Key Register Change |
|---|-------------|----------------------------------------|------------------------------------------------------|---------------------|
| 1 | 0x1000015c  | `mov.w r0, #0xd0000000`               | Loads SIO base address into r0                       | r0 = 0xd0000000     |
| 2 | 0x10000160  | `ldr r0, [r0, #0]`                    | Reads CPUID register at SIO base + 0                 | r0 = 0 (Core 0)     |
| 3 | 0x10000162  | `cbz r0, 0x1000016a`                  | If CPUID == 0 (Core 0), branch to data copy section  | PC = 0x1000016a     |
| 4 | 0x1000016a  | `ldr r0, [pc, #...]`                  | Loads source address for data copy (flash)           | r0 = flash addr     |
| 5 | 0x1000016c  | `ldr r1, [pc, #...]`                  | Loads destination address for data copy (RAM)        | r1 = RAM addr       |
| 6 | 0x1000016e  | `ldr r2, [pc, #...]`                  | Loads end address for data copy                      | r2 = end addr       |
| 7 | 0x10000170  | `cmp r1, r2`                          | Checks if all data has been copied                   | Flags updated       |
| 8 | 0x10000172  | `bhs 0x10000178`                      | If done (dest >= end), skip to BSS clear             | PC conditional      |
| 9 | 0x10000174  | `ldm r0!, {r3}`                       | Loads 4 bytes from flash source, auto-increments r0  | r3 = data, r0 += 4  |
| 10| 0x10000176  | `stm r1!, {r3}`                       | Stores 4 bytes to RAM destination, auto-increments r1| r1 += 4             |

##### GDB Session

```gdb
(gdb) b *0x1000015c
(gdb) monitor reset halt
(gdb) c
Breakpoint 1, 0x1000015c in _reset_handler ()
(gdb) si
(gdb) disas $pc,+2
(gdb) info registers r0
```

#### Reflection Answers

1. **Why does the reset handler check the CPUID before doing anything else?**
   The RP2350 has two Cortex-M33 cores that share the same reset handler entry point. The CPUID check at SIO base `0xd0000000` returns 0 for Core 0 and 1 for Core 1. Only Core 0 should perform the one-time initialization (data copy, BSS clear, calling `main()`). Without this check, both cores would simultaneously try to initialize RAM, causing data corruption and race conditions.

2. **What would happen if Core 1 tried to run the same initialization code as Core 0?**
   Both cores would simultaneously write to the same RAM locations during the data copy and BSS clear phases. This would cause data corruption due to race conditions—values could be partially written or overwritten unpredictably. The `cbz` instruction at `0x10000162` prevents this: if CPUID != 0, the code branches to `hold_non_core0_in_bootrom`, which sends Core 1 back to the bootrom to wait until Core 0 explicitly launches it later.

3. **Which registers are used in the first 10 instructions, and why those specific ones?**
   The first 10 instructions use `r0`, `r1`, `r2`, and `r3`. These are the ARM "caller-saved" scratch registers (r0–r3) that don't need to be preserved across function calls. Since the reset handler is the very first code to run (no caller to preserve state for), using these low registers is both efficient (16-bit Thumb encodings) and safe. `r0` handles CPUID check and source pointer, `r1` is the destination pointer, `r2` is the end marker, and `r3` is the data transfer register.

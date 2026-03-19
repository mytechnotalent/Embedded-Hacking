# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 6
Static Variables in Embedded Systems: Debugging and Hacking Static Variables w/ GPIO Input Basics

### Non-Credit Practice Exercise 2 Solution: Reverse Engineer gpio_set_pulls with GDB

#### Answers

##### Function Arguments

```gdb
(gdb) b *0x100002d8
(gdb) c
(gdb) info registers r0 r1 r2
r0 = 15   ; GPIO pin
r1 = 1    ; pull-up enable (true)
r2 = 0    ; pull-down disable (false)
```

##### PADS_BANK0 Address Calculation

```
Base: 0x40038000 (PADS_BANK0)
GPIO 0 offset: 0x04 (first pad register)
GPIO N offset: 0x04 + (N × 4)
GPIO 15: 0x40038000 + 0x04 + (15 × 4) = 0x40038000 + 0x04 + 0x3C = 0x40038040
```

##### Function Behavior Summary

| Step                    | Instruction(s)       | Effect                            |
|------------------------|---------------------|-----------------------------------|
| Load PADS base address | `ldr rX, =0x40038000` | rX = PADS_BANK0 base            |
| Calculate pad offset   | `adds`, `lsls`       | offset = 0x04 + pin × 4          |
| Read current config    | `ldr rX, [addr]`     | Read existing pad register value  |
| Clear pull bits        | `bic rX, rX, #0xC`   | Clear bits 2 (PDE) and 3 (PUE)   |
| Set pull-up bit        | `orr rX, rX, #0x8`   | Set bit 3 (PUE = pull-up enable) |
| Write back             | `str rX, [addr]`     | Write updated config to hardware  |

##### Pad Register Bits

| Bit | Name     | Value | Meaning                 |
|-----|----------|-------|-------------------------|
| 3   | PUE      | 1     | Pull-up enable          |
| 2   | PDE      | 0     | Pull-down disable       |
| 1   | SCHMITT  | 1     | Schmitt trigger enabled |
| 0   | SLEWFAST | 0     | Slow slew rate          |

#### Reflection Answers

1. **Why does the function read-modify-write the register instead of just writing a new value? What would happen if it just wrote without reading first?**
   The pad register contains multiple configuration fields (drive strength, slew rate, Schmitt trigger, input enable, output disable, pull-up, pull-down). If the function wrote a new value without reading first, it would overwrite all other fields with zeros or arbitrary values, potentially disabling the Schmitt trigger, changing drive strength, or disabling input/output. The read-modify-write pattern uses `bic` to clear only the pull bits (2 and 3) and `orr` to set the desired pull configuration, leaving all other bits untouched.

2. **The pad register for GPIO 15 is at offset `0x40` from the PADS base. How is this offset calculated? What would the offset be for GPIO 0?**
   The formula is: offset = `0x04 + (GPIO_number × 4)`. For GPIO 15: `0x04 + (15 × 4) = 0x04 + 0x3C = 0x40`. The `0x04` initial offset exists because offset `0x00` is the VOLTAGE_SELECT register, not a pad register. For GPIO 0: offset = `0x04 + (0 × 4) = 0x04`. So GPIO 0's pad register is at `0x40038004`.

3. **What would happen if you enabled BOTH pull-up and pull-down at the same time (bits 2 and 3 both set)?**
   Enabling both creates a **resistive voltage divider** between VDD and GND through the internal pull resistors. On the RP2350, both pull resistors are typically ~50kΩ. The pin voltage would settle at approximately VDD/2 (1.65V for 3.3V supply), which is in the undefined region between logic HIGH and LOW thresholds. This makes the digital input unreliable—the Schmitt trigger may oscillate or read randomly. While not damaging to the hardware, it wastes power and produces unpredictable input reads. The SDK intentionally never sets both bits simultaneously.

4. **The compiler inlines `gpio_pull_up` into `gpio_set_pulls`. What does this tell you about the compiler's optimization level? How does inlining affect binary analysis?**
   This indicates at least `-O1` or higher optimization (the Pico SDK defaults to `-O2`). The `gpio_pull_up` function is declared `static inline` in the SDK header, and the compiler eliminates the function call overhead by inserting `gpio_set_pulls(pin, true, false)` directly. For binary analysis, inlining means: (a) the original function name `gpio_pull_up` doesn't appear in the symbol table, (b) you see `gpio_set_pulls` called directly with hardcoded arguments, making it harder to identify the programmer's original intent, and (c) multiple calls to `gpio_pull_up` with different pins each become separate `gpio_set_pulls` calls rather than referencing a single function body.

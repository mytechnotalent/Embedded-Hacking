# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 11
Functions in Embedded Systems: Debugging and Hacking Functions w/ IR Remote and Multi-LED Control

### Non-Credit Practice Exercise 2 Solution: Change Blink Count

#### Answers

##### Blink Count Parameter

| Parameter    | Original | Patched |
|-------------|---------|---------|
| Blink count  | 3        | 5       |
| Hex          | 0x03     | 0x05    |
| Register     | r1       | r1      |
| Instruction  | movs r1, #3 | movs r1, #5 |

##### Assembly Context (blink_led Call)

```asm
movs r0, <pin>       ; r0 = GPIO pin number
movs r1, #3          ; r1 = blink count ← PATCH THIS
movs r2, #0x32       ; r2 = delay (50ms)
bl   blink_led       ; blink_led(pin, 3, 50)
```

##### Patch

The immediate byte in `movs r1, #3` is the first byte of the 2-byte Thumb instruction:

```
Before: 03 21  (movs r1, #3)
After:  05 21  (movs r1, #5)
```

File offset = instruction address - 0x10000000.

##### Behavior After Patch

| Button | LED    | Original          | Patched             |
|--------|--------|-------------------|---------------------|
| 1      | Red    | Blinks 3×, stays on | Blinks 5×, stays on |
| 2      | Green  | Blinks 3×, stays on | Blinks 5×, stays on |
| 3      | Yellow | Blinks 3×, stays on | Blinks 5×, stays on |

Total blink time at 50ms delay: 5 × (50 + 50) = **500ms** (was 300ms).

#### Reflection Answers

1. **movs rN, #imm8 can encode values 0–255. What is the maximum blink count with a single byte patch?**
   The maximum is **255** (`0xFF`). The `movs Rd, #imm8` Thumb instruction uses a full 8-bit immediate field, giving an unsigned range of 0–255. Setting the blink count to 255 would make each LED blink 255 times per button press — at 50ms on + 50ms off per blink, that's 255 × 100ms = **25.5 seconds** of blinking before the LED stays on. A count of 0 would skip the blink loop entirely (LED turns on immediately with no blinking).

2. **Why is blink count in r1 and not r0? What does r0 hold at this point?**
   The ARM calling convention (AAPCS) passes the first four function arguments in registers `r0`, `r1`, `r2`, `r3` in order. The `blink_led` function signature is `blink_led(uint8_t pin, uint8_t count, uint32_t delay_ms)`. So `r0` = pin (the GPIO number of the LED to blink), `r1` = count (how many times to blink), and `r2` = delay_ms (the delay in milliseconds between on/off transitions). The blink count is the second parameter, hence `r1`.

3. **If you wanted a blink count larger than 255 (e.g., 1000), what instruction sequence would the compiler generate instead of movs?**
   For values exceeding 255, the compiler would use a 32-bit Thumb-2 `movw r1, #imm16` instruction, which can encode 0–65535. For example, `movw r1, #1000` would be 4 bytes: `40 F2 E8 31` (encoding `movw r1, #0x3E8`). For values exceeding 65535, the compiler would add `movt r1, #imm16` to set the upper 16 bits, or use a literal pool load (`ldr r1, [pc, #offset]`). The function parameter type (`uint8_t`) would still truncate to 0–255, so a count of 1000 would wrap to 232 (1000 mod 256) unless the function uses a wider type internally.

4. **Is there one shared movs r1, #3 instruction for all three LEDs, or does each blink_led call have its own? How can you tell?**
   Each `blink_led` call likely has its **own** `movs r1, #3` instruction. The compiler generates separate parameter setup sequences for each `bl blink_led` call site — the `movs r0, <pin>` instruction before each call loads a different GPIO pin. You can verify by disassembling the full range (`disassemble 0x10000234,+300`) and counting how many `movs r1, #3` instructions appear before `bl blink_led` calls. If there are three separate call sites with three separate `movs r1, #3` instructions, you need to **patch all three** to change the blink count for every LED. If only one is patched, only that LED's blink count changes.

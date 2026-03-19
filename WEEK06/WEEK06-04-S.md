# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 6
Static Variables in Embedded Systems: Debugging and Hacking Static Variables w/ GPIO Input Basics

### Non-Credit Practice Exercise 4 Solution: Invert the Button Logic with XOR

#### Answers

##### Patch Details

| Item                | Original              | Patched               |
|--------------------|-----------------------|-----------------------|
| Instruction        | eor.w r3, r3, #1     | eor.w r3, r3, #0     |
| Address            | 0x10000286            | 0x10000286            |
| Hex bytes          | 83 F0 01 03           | 83 F0 00 03           |
| Patched byte offset| 0x288 (3rd byte)      | 01 → 00               |

##### Logic Table Comparison

**Original (EOR #1):**

| Button State | GPIO 15 | After UBFX | After EOR #1 | LED (GPIO 16) |
|-------------|---------|------------|-------------|---------------|
| Released    | 1 (HIGH)| 1          | 0           | OFF           |
| Pressed     | 0 (LOW) | 0          | 1           | ON            |

**Patched (EOR #0):**

| Button State | GPIO 15 | After UBFX | After EOR #0 | LED (GPIO 16) |
|-------------|---------|------------|-------------|---------------|
| Released    | 1 (HIGH)| 1          | 1           | **ON**        |
| Pressed     | 0 (LOW) | 0          | 0           | **OFF**       |

##### Hardware Result

- Button NOT pressed: LED **ON** (was OFF)
- Button PRESSED: LED **OFF** (was ON)
- Behavior completely reversed by changing a single byte (01 → 00)

#### Reflection Answers

1. **Why does XOR with 1 act as a NOT for single-bit values? Write out the truth table for `x XOR 1` and `x XOR 0` where x is 0 or 1.**

   | x | x XOR 1 | x XOR 0 |
   |---|---------|---------|
   | 0 | 1       | 0       |
   | 1 | 0       | 1       |

   `x XOR 1` always flips the bit (acts as NOT): 0→1, 1→0. `x XOR 0` always preserves the bit (acts as identity): 0→0, 1→1. This works because XOR returns 1 when inputs differ and 0 when they match. XOR with 1 forces a difference; XOR with 0 forces a match. This property only applies to the single affected bit—for multi-bit values, each bit is XORed independently.

2. **Instead of changing `eor.w r3, r3, #1` to `eor.w r3, r3, #0`, could you achieve the same result by NOPing (removing) the instruction entirely? What bytes encode a NOP in Thumb?**
   Yes. Removing the EOR instruction entirely would have the same effect as EOR #0—the value passes through unchanged. A Thumb NOP is encoded as `00 BF` (2 bytes). Since `eor.w` is a 32-bit Thumb-2 instruction (4 bytes), you would need **two** NOPs to replace it: `00 BF 00 BF`. In the hex editor, replace bytes at offset 0x286–0x289 from `83 F0 01 03` to `00 BF 00 BF`. Both approaches yield identical behavior, but the EOR #0 patch is "cleaner" because it preserves the instruction structure—making the modification more obvious during reverse engineering.

3. **The pull-up resistor means "pressed = LOW." If you removed the pull-up (changed `gpio_pull_up` to no pull), would the button still work? Why or why not?**
   It would be unreliable. Without a pull-up or pull-down resistor, the GPIO input is **floating** when the button is not pressed—there's no defined voltage on the pin. The input would pick up electrical noise, stray capacitance, and electromagnetic interference, causing random readings (0 or 1 unpredictably). When the button IS pressed, it connects to ground (LOW), which works. But when released, the pin has no path to any voltage, so `gpio_get(15)` returns garbage. The pull-up provides a defined HIGH state when the button circuit is open.

4. **The `ubfx r3, r3, #0xf, #0x1` instruction extracts bit 15. If you changed `#0xf` to `#0x10` (bit 16), what GPIO pin would you be reading? What value would you get if nothing is connected to that pin?**
   You would be reading **GPIO 16**, which is the LED output pin. Since GPIO 16 is configured as an output (not input), reading its input register returns the current output state—either 0 or 1 depending on whether the LED is currently on or off. This would create a **feedback loop**: the LED's current state determines its next state (after the XOR), causing unpredictable oscillation or a stuck state. If GPIO 16 had nothing connected and was unconfigured, the floating input would return random values, similar to Q3's scenario.

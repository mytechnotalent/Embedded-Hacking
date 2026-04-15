# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 5
Integers and Floats in Embedded Systems: Debugging and Hacking Integers and Floats w/ Intermediate GPIO Output Assembler Analysis

### Non-Credit Practice Exercise 2 Solution: Patch the Float Binary — Changing 42.5 to 99.0

#### Answers

##### IEEE 754 Encoding of 99.0

```
Integer: 99 = 1100011₂
Fractional: .0 = .0₂
Combined: 1100011.0₂
Normalized: 1.100011₂ × 2⁶

Sign: 0 (positive)
Exponent: 6 + 1023 = 1029 = 10000000101₂
Mantissa: 100011 followed by 46 zeros

Full double: 0x4058C00000000000
```

##### Patch Summary

| Register | Old Value (42.5) | New Value (99.0) | Changed? |
|----------|-----------------|------------------|----------|
| r2       | 0x00000000      | 0x00000000       | No       |
| r3       | 0x40454000      | 0x4058C000       | **Yes**  |

##### Ghidra Patch

```
DAT_1000024c:
  Before (little-endian): 00 40 45 40  → 0x40454000
  After  (little-endian): 00 C0 58 40  → 0x4058C000
```

##### Serial Output

```
fav_num: 99.000000
fav_num: 99.000000
fav_num: 99.000000
...
```

#### Reflection Answers

1. **Why did we only need to patch r3 (the high word) and not r2 (the low word)?**
   Both 42.5 and 99.0 have "clean" fractional parts that can be exactly represented with few mantissa bits. For 42.5, the mantissa is `010101000...0`; for 99.0, it's `100011000...0`. In both cases, all significant mantissa bits fit within the top 20 bits (stored in r3 bits 19–0), leaving the bottom 32 bits (r2) as all zeros. Only values with complex or repeating binary fractions (like 42.52525 or 99.99) need non-zero low words.

2. **What would the high word be if we wanted to patch the value to `-99.0` instead?**
   Flip bit 31 of r3 (the sign bit). The current r3 = `0x4058C000` = `0100 0000 0101 1000 1100...`. Setting bit 31 to 1: `0xC058C000` = `1100 0000 0101 1000 1100...`. The full double encoding of −99.0 is `0xC058C00000000000`. Only the most significant nibble changes from `4` to `C`.

3. **Walk through the encoding of `100.0` as a double. What are the high and low words?**
   ```
   100 = 1100100₂
   100.0 = 1100100.0₂ = 1.1001₂ × 2⁶
   Sign: 0
   Exponent: 6 + 1023 = 1029 = 10000000101₂
   Mantissa: 1001 followed by 48 zeros
   
   Full 64-bit: 0 10000000101 1001000000...0
   High word (r3): 0x40590000
   Low word (r2):  0x00000000
   ```
   Verification: `struct.pack('>d', 100.0).hex()` → `4059000000000000` ✓

4. **Why do we need the `--family 0xe48bff59` flag when converting to UF2?**
   The `--family` flag specifies the target chip family in the UF2 file header. `0xe48bff59` is the registered family ID for the RP2350. The bootloader reads this field to verify the firmware is intended for the correct chip before flashing. If the family ID doesn't match (e.g., using the RP2040 ID `0xe48bff56`), the bootloader may reject the firmware or write it incorrectly. This prevents accidentally flashing RP2040 firmware onto an RP2350 (or vice versa), which could cause undefined behavior since the chips have different architectures (Cortex-M0+ vs Cortex-M33).

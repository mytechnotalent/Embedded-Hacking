# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 5
Integers and Floats in Embedded Systems: Debugging and Hacking Integers and Floats w/ Intermediate GPIO Output Assembler Analysis

### Non-Credit Practice Exercise 4 Solution: Patch the Double Binary — Changing 42.52525 to 99.99

#### Answers

##### IEEE 754 Encoding of 99.99

```
Integer:    99 = 1100011₂
Fractional: .99 ≈ .111111010111...₂ (repeating)
Combined:   1100011.111111010111...₂
Normalized: 1.100011111111010111...₂ × 2⁶

Sign: 0 (positive)
Exponent: 6 + 1023 = 1029 = 10000000101₂
Mantissa: 1000111111110101 11000010100011110101 11000010100011110...₂  (52 bits)

Full double: 0x4058FF5C28F5C28F
```

Python verification: `struct.pack('>d', 99.99).hex()` → `4058ff5c28f5c28f` ✓

##### Patch Summary

| Register | Old Value (42.52525) | New Value (99.99) | Changed? |
|----------|---------------------|-------------------|----------|
| r2       | 0x645A1CAC          | 0x28F5C28F        | **Yes**  |
| r3       | 0x4045433B          | 0x4058FF5C        | **Yes**  |

##### Ghidra Patches

**Low word (DAT_10000254):**
```
Before (little-endian): AC 1C 5A 64  → 0x645A1CAC
After  (little-endian): 8F C2 F5 28  → 0x28F5C28F
```

**High word (DAT_10000258):**
```
Before (little-endian): 3B 43 45 40  → 0x4045433B
After  (little-endian): 5C FF 58 40  → 0x4058FF5C
```

##### Serial Output

```
fav_num: 99.990000
fav_num: 99.990000
fav_num: 99.990000
...
```

#### Reflection Answers

1. **Why did both r2 and r3 change when patching 42.52525 → 99.99, but only r3 changed when patching 42.5 → 99.0?**
   Both 42.5 and 99.0 have "clean" fractional parts (0.5 and 0.0 respectively) that are exact in binary—they need very few mantissa bits, all fitting in the top 20 bits of r3. The low word (r2) remains `0x00000000` for both. In contrast, 42.52525 and 99.99 both have repeating binary fractions (0.52525 and 0.99 respectively) that require all 52 mantissa bits to approximate. Since the low 32 bits of the mantissa live in r2, changing from one repeating fraction to another necessarily changes both r2 and r3.

2. **The multiply-by-2 method for 0.99 produces a repeating pattern. What does this mean for the precision of the stored value?**
   It means 99.99 **cannot** be represented exactly as an IEEE 754 double. The binary fraction 0.111111010111... repeats indefinitely, but the mantissa only has 52 bits. The stored value is the closest 52-bit approximation, which is 99.98999999999999... (off by approximately 10⁻¹⁴). This is a fundamental limitation of binary floating-point: decimal fractions that aren't sums of negative powers of 2 always produce repeating binary expansions. The `printf` output rounds to `99.990000` because the default `%lf` precision (6 decimal places) hides the tiny error.

3. **If you wanted to patch the double to `100.0` instead of `99.99`, how many data constants would need to change?**
   **Both** would need to change—but for the opposite reason. Currently r2 = `0x645A1CAC` (non-zero). For 100.0: `struct.pack('>d', 100.0).hex()` = `4059000000000000`, so r3 = `0x40590000` and r2 = `0x00000000`. The r2 constant must be patched from `0x645A1CAC` to `0x00000000`, and r3 from `0x4045433B` to `0x40590000`. Even though the low word becomes zero, you still need to patch it because it was previously non-zero.

4. **Compare the Ghidra Listing for the float binary (Exercise 1) and the double binary (Exercise 3). How does the compiler load the double differently?**
   The float binary uses separate instructions: `movs r4, #0x0` (loads zero into r4 for the low word) and `ldr r5, [DAT_1000024c]` (loads the high word from a literal pool). The double binary uses a single `ldrd r4, r5, [r5, #0x0]` instruction that loads both words from consecutive memory addresses in one operation. The `ldrd` approach is more efficient (fewer instructions, single memory transaction) and is preferred when both words carry meaningful data. The float's approach works fine because one word is a trivially loaded zero.

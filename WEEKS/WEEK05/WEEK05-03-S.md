# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 5
Integers and Floats in Embedded Systems: Debugging and Hacking Integers and Floats w/ Intermediate GPIO Output Assembler Analysis

### Non-Credit Practice Exercise 3 Solution: Analyze the Double Binary in Ghidra

#### Answers

##### Register Pair for 42.52525

| Register | Value        | Role          |
|----------|-------------|---------------|
| r2       | 0x645A1CAC  | Low 32 bits   |
| r3       | 0x4045433B  | High 32 bits  |

Full double: **0x4045433B645A1CAC**

##### IEEE 754 Decoding

```
r3 = 0x4045433B = 0100 0000 0100 0101 0100 0011 0011 1011
r2 = 0x645A1CAC = 0110 0100 0101 1010 0001 1100 1010 1100

Sign bit (bit 63):     0 → Positive
Exponent (bits 62-52): 10000000100 = 1028 → 1028 - 1023 = 5
Mantissa (bits 51-0):  0101010000110011101101100100010110100001110010101100

Value = 1.0101010000110011...₂ × 2⁵ = 101010.10000110011...₂
Integer part: 101010₂ = 32 + 8 + 2 = 42
Fractional part: .10000110011... ≈ 0.52525
Result: 42.52525 ✓
```

##### Float vs Double Comparison

| Item                      | Float (42.5)         | Double (42.52525)       |
|--------------------------|---------------------|------------------------|
| r2 (low word)            | 0x00000000          | 0x645A1CAC             |
| r3 (high word)           | 0x40454000          | 0x4045433B             |
| Low word is zero?        | Yes                 | **No**                 |
| Words to patch           | 1 (r3 only)         | **2 (both r2 and r3)** |
| Format specifier         | %f                  | %lf                    |
| Assembly load instruction| movs + ldr          | ldrd (load double)     |

##### Key Assembly

```assembly
10000238    push    {r3,r4,r5,lr}
1000023a    adr     r5, [0x10000254]
1000023c    ldrd    r4, r5, [r5, #0x0]    ; r4 = 0x645A1CAC, r5 = 0x4045433B
10000240    bl      stdio_init_all
10000244    mov     r2, r4                 ; r2 = low word
10000246    mov     r3, r5                 ; r3 = high word
```

#### Reflection Answers

1. **Why does `42.5` have a zero low word but `42.52525` does not?**
   The fractional part determines whether the low word is zero. 0.5 in binary is exactly 2⁻¹ = `0.1₂`—a single bit. After normalization, the mantissa for 42.5 is `010101000...0`, needing only 6 significant bits, all fitting in the top 20 mantissa bits within r3. In contrast, 0.52525 is a **repeating binary fraction** that cannot be represented exactly—it requires all 52 mantissa bits to approximate as closely as possible. The lower 32 bits in r2 (`0x645A1CAC`) carry the additional precision needed for this approximation.

2. **The assembly uses `ldrd r4, r5, [r5, #0x0]` instead of two separate `ldr` instructions. What is the advantage?**
   `ldrd` (Load Register Double) loads two consecutive 32-bit words from memory in a single instruction, completing in one memory access cycle (or two back-to-back aligned accesses on the bus). Using two separate `ldr` instructions would require two instruction fetches, two decode cycles, and two memory accesses. `ldrd` reduces code size by 4 bytes (one 4-byte instruction vs. two) and improves performance by allowing the memory controller to pipeline both loads. For 64-bit doubles that are always loaded in pairs, `ldrd` is the optimal choice.

3. **Both the float and double programs have the same exponent (stored as 1028, real exponent 5). Why?**
   Both 42.5 and 42.52525 fall in the same range: between 32 (2⁵) and 64 (2⁶). Normalization produces `1.xxx × 2⁵` for both values. The exponent is determined solely by the magnitude (which power of 2 the number falls between), not by the fractional precision. Any number from 32.0 to 63.999... would have real exponent 5 (stored as 1028). The mantissa captures the differences—42.5 has mantissa `010101000...` while 42.52525 has `0101010000110011...`.

4. **If you were patching this double, how many data constants would you need to modify compared to the float exercise?**
   **Two** data constants—both `DAT_10000254` (low word, r2) and `DAT_10000258` (high word, r3). In the float exercise (42.5), only one constant needed patching because r2 was zero and stayed zero when patching to 99.0. For the double 42.52525, since the low word is already non-zero (`0x645A1CAC`), any new value with a different repeating fraction will require changing both words. The only exception would be patching to a value whose low word happens to also be `0x645A1CAC` (virtually impossible for an arbitrary target value).

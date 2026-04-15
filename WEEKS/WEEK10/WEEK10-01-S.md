# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 10
Dynamic Conditionals in Embedded Systems: Debugging and Hacking Dynamic Conditionals w/ SG90 Servo Basics

### Non-Credit Practice Exercise 1 Solution: Change Servo Angle Range

#### Answers

##### IEEE-754 Angle Encodings

| Angle  | IEEE-754 Hex  | Little-Endian Bytes | Sign | Exponent | Mantissa  |
|--------|--------------|--------------------|----- |----------|-----------|
| 0.0f   | 0x00000000   | 00 00 00 00        | 0    | 0        | 0         |
| 45.0f  | 0x42340000   | 00 00 34 42        | 0    | 132      | 0x340000  |
| 135.0f | 0x43070000   | 00 00 07 43        | 0    | 134      | 0x070000  |
| 180.0f | 0x43340000   | 00 00 34 43        | 0    | 134      | 0x340000  |

##### Patch 1: Maximum Angle 180.0f → 135.0f

```
Before: 00 00 34 43  (180.0f)
After:  00 00 07 43  (135.0f)
```

##### Patch 2: Minimum Angle 0.0f → 45.0f

```
Before: 00 00 00 00  (0.0f)
After:  00 00 34 42  (45.0f)
```

##### Result

Servo sweeps from **45° to 135°** instead of 0° to 180°, a 90° range centered at the midpoint.

##### Conversion and Flash

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0020_dynamic-conditionals
python ..\uf2conv.py build\0x0020_dynamic-conditionals-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

#### Reflection Answers

1. **Break down the IEEE-754 encoding of 180.0f (0x43340000). What are the sign bit, exponent, and mantissa fields?**
   `0x43340000` in binary: `0 10000110 01101000000000000000000`. **Sign** = 0 (positive). **Exponent** = `10000110` = 134, biased exponent = 134 - 127 = 7. **Mantissa** = `0x340000` = `01101000...0`, representing 1.01101₂ = 1 + 0.25 + 0.125 + 0.03125 = 1.40625. Value = 1.40625 × 2⁷ = 1.40625 × 128 = **180.0**.

2. **Why is 0.0f represented as 0x00000000 (all zeros) in IEEE-754? Most floats have a non-zero exponent — what makes zero special?**
   Zero is a **special case** in IEEE-754. When both the exponent and mantissa are all zeros, the value is defined as ±0.0 (the sign bit distinguishes +0.0 from -0.0). This is by design — the IEEE-754 standard reserves the all-zeros exponent for zero and denormalized numbers. Unlike normal floats that have an implicit leading 1 in the mantissa (1.xxx), zero has no such implicit bit. This special encoding means you can check for zero by testing if all 32 bits are 0, which is efficient for hardware.

3. **What is the IEEE-754 encoding of 90.0f? Show the sign, exponent, and mantissa calculation.**
   90.0 = 1.40625 × 2⁶. **Sign** = 0. **Exponent** = 6 + 127 = 133 = `0x85` = `10000101`. **Mantissa**: 90.0 / 64 = 1.40625, fractional part = 0.40625 = 0.25 + 0.125 + 0.03125 = `0110100...0` = `0x340000`. Result: `0 10000101 01101000000000000000000` = `0x42B40000`. Little-endian: `00 00 B4 42`.

4. **The compiler might use movs r0, #0 instead of loading 0.0f from a literal pool. Why would it choose one approach over the other?**
   For integer zero, the compiler prefers `movs r0, #0` (a 2-byte Thumb instruction) because it's smaller and faster than a literal pool load. However, for **floating-point** zero used with VFP instructions, the compiler must load it into an FPU register (e.g., `s0`). If the FPU has a `vmov.f32 s0, #0.0` immediate form available, it can encode zero directly. Otherwise, it loads from a literal pool or uses `movs` to set an integer register to 0 and transfers it with `vmov s0, r0`. The choice depends on instruction context — integer vs. FPU register — and optimization level.

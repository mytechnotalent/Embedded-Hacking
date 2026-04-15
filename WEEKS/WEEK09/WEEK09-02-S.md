# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 9
Operators in Embedded Systems: Debugging and Hacking Operators w/ DHT11 Basics

### Non-Credit Practice Exercise 2 Solution: Invert the Temperature Reading

#### Answers

##### IEEE-754 Sign Bit Flip

| Value   | IEEE-754 Hex  | Little-Endian Bytes | Sign Bit |
|---------|--------------|--------------------|---------| 
| +0.1f   | 0x3DCCCCCD   | CD CC CC 3D        | 0        |
| -0.1f   | 0xBDCCCCCD   | CD CC CC BD        | 1        |

Only the **last byte** changes in little-endian: `3D` → `BD`. This flips bit 31 (the IEEE-754 sign bit) from 0 to 1, negating the value.

##### Patch at File Offset 0x42C

```
Offset 0x42C:
Before: CD CC CC 3D  (+0.1f)
After:  CD CC CC BD  (-0.1f)
```

Only 1 byte changes: offset `0x42F` from `0x3D` to `0xBD`.

##### How the Temperature Changes

The DHT11 returns raw integer and decimal parts (e.g., integer=24, decimal=0). The firmware computes:

```
temperature = integer_part + (decimal_part × 0.1f)
```

With `vfma.f32 s15, s13, s11`: result = s15 + (s13 × s11) = integer + (decimal × 0.1f)

After patching to -0.1f: result = integer + (decimal × -0.1f)

For a reading of 24.5°C: original = 24 + (5 × 0.1) = 24.5°C, patched = 24 + (5 × -0.1) = 23.5°C

For a reading of 24.0°C: integer=24, decimal=0, so 24 + (0 × -0.1) = 24.0°C (unchanged when decimal is 0).

##### Serial Output After Patch

```
Humidity: 51.0%, Temperature: 23.5°C
```

(Temperature decimal contribution is inverted; effect depends on the decimal component.)

#### Reflection Answers

1. **Why does changing the byte from 0x3D to 0xBD negate the float? What specific bit is being flipped?**
   In IEEE-754 single-precision format, bit 31 is the **sign bit**: 0 = positive, 1 = negative. The byte `0x3D` in binary is `0011 1101` and `0xBD` is `1011 1101` — only bit 7 of that byte differs, which corresponds to bit 31 of the 32-bit float (since it's the MSB of the last byte in little-endian storage). The exponent and mantissa bits remain identical, so the magnitude stays exactly `0.1` — only the sign changes.

2. **Why does patching one constant affect both humidity AND temperature? They use different vfma instructions (at 0x410 and 0x414) — so why are both affected?**
   Both `vfma` instructions at `0x410` (humidity) and `0x414` (temperature) load the scaling constant from the **same literal pool entry** at offset `0x42C`. The compiler recognized that both computations use the same `0.1f` value and stored it only once to save space. Both `vldr s11, [pc, #offset]` instructions resolve to address `0x1000042C`. So patching that single 4-byte value changes the scaling factor for both humidity and temperature simultaneously.

3. **What is the IEEE-754 encoding of 0.5f? If the raw sensor decimal reading was 8, what would the computed value be with 0.5f instead of 0.1f?**
   0.5f in IEEE-754: sign=0, exponent=126 (`0x7E`), mantissa=0. Hex = `0x3F000000`. Little-endian bytes: `00 00 00 3F`. With a raw decimal reading of 8: `8 × 0.5 = 4.0`. So if the integer part was 24, the result would be `24 + 4.0 = 28.0°C` instead of `24 + 0.8 = 24.8°C`.

4. **Could you achieve the same inversion by patching the vfma instruction instead of the constant? What instruction change would work?**
   Yes. You could change `vfma.f32` (fused multiply-add: d = d + a×b) to `vfms.f32` (fused multiply-subtract: d = d - a×b). This would compute `temperature = integer - (decimal × 0.1f)` instead of `integer + (decimal × 0.1f)`, achieving the same sign inversion on the decimal contribution. The instruction encoding difference between `vfma` and `vfms` is typically a single bit in the opcode. However, this approach is more complex than simply flipping the sign bit of the constant.

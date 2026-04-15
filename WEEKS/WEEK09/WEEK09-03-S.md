# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 9
Operators in Embedded Systems: Debugging and Hacking Operators w/ DHT11 Basics

### Non-Credit Practice Exercise 3 Solution: Add a Fixed Temperature Offset

#### Answers

##### Two Patches Required

This exercise requires **two** simultaneous patches to add a fixed +10°C offset to every temperature reading.

##### Patch 1: Instruction at Offset 0x414

Change `vfma.f32` to `vadd.f32`:

| Item        | Original                      | Patched                       |
|------------|------------------------------|------------------------------|
| Instruction | vfma.f32 s15, s13, s11       | vadd.f32 s15, s15, s11       |
| Encoding    | e6 ee a5 7a                  | b4 ee a5 7a                  |
| Operation   | s15 = s15 + (s13 × s11)     | s15 = s15 + s11              |

```
Offset 0x414:
Before: E6 EE A5 7A  (vfma.f32)
After:  B4 EE A5 7A  (vadd.f32)
```

Only the first two bytes change: `E6 EE` → `B4 EE`.

##### Patch 2: Constant at Offset 0x42C

Change the constant from 0.1f to 10.0f:

| Value  | IEEE-754 Hex  | Little-Endian Bytes |
|--------|--------------|--------------------| 
| 0.1f   | 0x3DCCCCCD   | CD CC CC 3D        |
| 10.0f  | 0x41200000   | 00 00 20 41        |

```
Offset 0x42C:
Before: CD CC CC 3D  (0.1f)
After:  00 00 20 41  (10.0f)
```

##### Why Both Patches Are Needed

- **Original:** `vfma.f32` computes `temp = integer + (decimal × 0.1f)` — fractional scaling
- **After both patches:** `vadd.f32` computes `temp = integer + 10.0f` — fixed offset addition
- **If only constant changed:** `vfma.f32` would compute `temp = integer + (decimal × 10.0f)` — amplified decimal, not a fixed offset

##### Serial Output After Patch

```
Humidity: 51.0%, Temperature: 34.0°C
```

(Original 24.0°C + 10.0°C offset = 34.0°C)

#### Reflection Answers

1. **Why are both patches needed? What would happen if you only changed the constant to 10.0f but left vfma unchanged?**
   If you only changed `0.1f` to `10.0f` but left `vfma.f32`, the computation would be `temp = integer + (decimal × 10.0f)`. For a reading of 24.5°C (integer=24, decimal=5): result = 24 + (5 × 10.0) = 74.0°C — wildly incorrect. The `vfma` instruction multiplies two operands and adds, so the constant serves as a multiplier for the decimal part. By changing to `vadd.f32`, we eliminate the multiplication entirely and just add the constant directly to the integer, giving `24 + 10.0 = 34.0°C`.

2. **The humidity vfma instruction at 0x410 was NOT changed. Both vfma instructions (0x410 and 0x414) load the same 0.1f constant from 0x42C. With the constant now 10.0f, what happens to the humidity computation?**
   The humidity `vfma` at `0x410` now computes `hum = integer + (decimal × 10.0f)`. If the humidity decimal part is 0 (e.g., raw humidity = 51.0%), then `51 + (0 × 10.0) = 51.0%` — unchanged. But if the decimal part is non-zero (e.g., raw = 51.3%, decimal=3), the result would be `51 + (3 × 10.0) = 81.0%` — grossly incorrect. The DHT11 sensor's humidity decimal is often 0, so you might not notice the bug immediately, but it's a latent defect.

3. **If you wanted to add a 10°C offset to temperature WITHOUT affecting humidity, what additional patch(es) would you need?**
   You would need to ensure humidity still uses the original `0.1f` scaling. Options: (1) Also change the humidity `vfma` at `0x410` to `vadd.f32` and create a separate literal pool entry with `0.1f` for it — but this requires finding free space. (2) More practically, place a second copy of `0.1f` (`CD CC CC 3D`) in unused space in the binary, and redirect the humidity `vldr` instruction's PC-relative offset to point to that new location instead of `0x42C`. (3) Alternatively, NOP out the humidity `vfma` entirely if the decimal contribution is negligible.

4. **Why do only the first 2 bytes differ between vfma and vadd? What do the last 2 bytes encode?**
   In the ARM VFPv4 encoding, the first two bytes (`E6 EE` vs `B4 EE`) contain the **opcode** that distinguishes the operation type (fused multiply-add vs addition). The last two bytes (`A5 7A`) encode the **operand registers**: the source and destination VFP registers (s15, s13, s11). Since both instructions operate on the same registers, the operand encoding is identical. Only the operation code changes — this is a characteristic of the ARM instruction set where opcode and operand fields are cleanly separated.

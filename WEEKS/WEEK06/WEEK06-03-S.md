# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 6
Static Variables in Embedded Systems: Debugging and Hacking Static Variables w/ GPIO Input Basics

### Non-Credit Practice Exercise 3 Solution: Make the Overflow Happen Faster

#### Answers

##### Patch Details

| Item               | Original         | Patched          |
|--------------------|-----------------|-----------------|
| Instruction        | adds r3, #0x1   | adds r3, #0xa   |
| Address            | 0x1000027c      | 0x1000027c      |
| Hex bytes          | 01 33           | 0A 33           |
| Increment value    | 1               | 10              |
| File offset        | 0x27c           | 0x27c           |

##### Instruction Encoding

```
Thumb adds rD, #imm8:
  Byte 0: immediate value (0x01 → 0x0A)
  Byte 1: opcode + register (0x33 = adds r3)
```

##### Serial Output After Patch

```
regular_fav_num: 42
static_fav_num: 42
regular_fav_num: 42
static_fav_num: 52
regular_fav_num: 42
static_fav_num: 62
...
regular_fav_num: 42
static_fav_num: 252
regular_fav_num: 42
static_fav_num: 6      ← Overflow! 252 + 10 = 262 mod 256 = 6
```

#### Reflection Answers

1. **The overflow now wraps to 6 instead of 0. Explain why, using the modular arithmetic of a `uint8_t` (range 0-255).**
   A `uint8_t` stores values modulo 256. Starting at 42 and incrementing by 10: the sequence passes through 42, 52, 62, ..., 242, 252. The next value is 252 + 10 = 262. Since `uint8_t` can only hold 0–255: $262 \bmod 256 = 6$. The wrap value is non-zero because the increment (10) does not evenly divide into 256. With increment 1, the value hits exactly 255, and $255 + 1 = 256 \bmod 256 = 0$. With increment 10, it skips from 252 directly to 262, bypassing 0 and landing on 6.

2. **What is the maximum value you could change the increment to while still using `adds r3, #imm8`? What would happen if you needed an increment larger than 255?**
   The maximum is **255** (`0xFF`). The `adds rD, #imm8` Thumb encoding has an 8-bit immediate field, so valid values are 0–255. For an increment larger than 255, you would need to: (a) use a 32-bit Thumb-2 `adds.w` instruction which supports a wider range of modified immediates, (b) use a `movs` + `adds` two-instruction sequence to load a larger value, or (c) load the value from a literal pool with `ldr` then use a register-register `add`. Each approach requires different instruction sizes, so patching in a hex editor becomes more complex—you may need to shift code or use NOP padding.

3. **If you changed the increment to 128 (`0x80`), how many iterations would it take to wrap, and what value would it wrap to?**
   Starting at 42, incrementing by 128: 42 → 170 → 42 → 170 → ... Wait, let's compute: $42 + 128 = 170$ (first iteration), $170 + 128 = 298 \bmod 256 = 42$ (second iteration). It wraps after **2 iterations** back to 42. The variable alternates between 42 and 170 forever because $2 \times 128 = 256$, and $42 + 256 = 42 \bmod 256$. The value never reaches 0—it cycles between exactly two values.

4. **Could you achieve the same speedup by changing the `strb` (store byte) to `strh` (store halfword)? Why or why not?**
   No. Changing `strb` to `strh` would store 16 bits instead of 8, which means the variable would be treated as a `uint16_t` (range 0–65535). This would actually **slow down** overflow—it would take 65,535 − 42 = 65,493 iterations to wrap instead of 213. Additionally, `strh` writes 2 bytes to RAM, potentially corrupting the adjacent byte at `0x200005a9`. The proper way to speed up overflow is to increase the increment value, not change the storage width. The `strb` truncation to 8 bits is what enforces the `uint8_t` modular arithmetic.

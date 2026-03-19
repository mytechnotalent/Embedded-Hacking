# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 9
Operators in Embedded Systems: Debugging and Hacking Operators w/ DHT11 Basics

### Non-Credit Practice Exercise 1 Solution: Change the Sleep Duration

#### Answers

##### Sleep Duration Values

| Parameter      | Original     | Patched      |
|---------------|-------------|-------------|
| Duration (ms)  | 2000         | 5000         |
| Hex            | 0x000007D0   | 0x00001388   |
| Little-endian  | D0 07 00 00  | 88 13 00 00  |

##### Why a Literal Pool Is Needed

The value 2000 exceeds the 8-bit immediate range of `movs` (0–255) and the 16-bit range is also impractical for a single-instruction load. The compiler stores `0x000007D0` in a **literal pool** near the function code and loads it with a `ldr r0, [pc, #offset]` instruction that reads the 32-bit word from the pool into `r0` before the `bl sleep_ms` call.

##### Patch Procedure

1. Find the literal pool entry containing `D0 07 00 00` in HxD
2. Replace with `88 13 00 00`

```
Before: D0 07 00 00  (2000)
After:  88 13 00 00  (5000)
```

##### Conversion and Flash

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x001a_operators
python ..\uf2conv.py build\0x001a_operators-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

##### Serial Output After Patch

```
arithmetic_operator: 50
increment_operator: 5
relational_operator: 0
logical_operator: 0
bitwise_operator: 12
assignment_operator: 11
Humidity: 51.0%, Temperature: 24.0°C
```

Output repeats every **5 seconds** instead of 2 seconds.

#### Reflection Answers

1. **Why can't 2000 be encoded as a movs immediate? What is the maximum value movs can hold?**
   The `movs Rd, #imm8` instruction is a 16-bit Thumb encoding that has only 8 bits for the immediate value, giving a range of 0–255. The value 2000 (`0x7D0`) is far beyond this range. Even the 32-bit Thumb-2 `movw` instruction can only encode 0–65535, which could handle 2000, but the compiler chose a literal pool approach. The literal pool is a general-purpose solution that works for any 32-bit value, including addresses and large constants.

2. **If you wanted to change the sleep to exactly 1 second (1000ms), what 4 bytes would you write in little-endian? Show your work.**
   1000 decimal = `0x000003E8` hex. In little-endian byte order (LSB first): `E8 03 00 00`. Breakdown: byte 0 = `0xE8` (LSB), byte 1 = `0x03`, byte 2 = `0x00`, byte 3 = `0x00` (MSB).

3. **Could other code in the program reference the same literal pool entry containing 0x7D0? What would happen if it did?**
   Yes, the compiler may share literal pool entries to save space. If another instruction also loads `0x7D0` from the same pool address (using its own `ldr rN, [pc, #offset]`), then patching that pool entry would change the value for ALL instructions that reference it. This is a risk with literal pool patching — you might unintentionally modify other parts of the program. To check, search for all `ldr` instructions whose PC-relative offset resolves to the same pool address.

4. **What would happen if you set sleep_ms to 0? Would the program crash or just run extremely fast?**
   The program would not crash — `sleep_ms(0)` is a valid call that returns immediately. The loop would run as fast as the processor can execute it, printing operator values and reading the DHT11 sensor with no delay between iterations. The serial output would flood extremely quickly. However, the DHT11 sensor has a minimum sampling interval of about 1 second; reading it more frequently may return stale data or read errors ("DHT11 read failed"), but the program itself would continue running.

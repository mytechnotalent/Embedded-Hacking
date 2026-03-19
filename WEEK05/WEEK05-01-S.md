# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 5
Integers and Floats in Embedded Systems: Debugging and Hacking Integers and Floats w/ Intermediate GPIO Output Assembler Analysis

### Non-Credit Practice Exercise 1 Solution: Analyze the Float Binary in Ghidra

#### Answers

##### Main Function Analysis

| Item                      | Value                  | Notes                              |
|--------------------------|------------------------|------------------------------------|
| Main function address    | 0x10000234             | Entry point of program             |
| Float value (original)   | 42.5                   | Declared as `float`                |
| Double hex encoding      | 0x4045400000000000     | Promoted to double for printf      |
| r3 (high word)           | 0x40454000             | Sign + exponent + top mantissa     |
| r2 (low word)            | 0x00000000             | All zeros (clean fractional part)  |
| Exponent (stored)        | 1028                   | Biased value                       |
| Exponent (real)          | 5                      | After subtracting bias 1023        |
| Format string            | "fav_num: %f\r\n"      | Located at 0x100034a8              |
| stdio_init_all address   | 0x10002f5c             | I/O initialization                 |
| printf address           | 0x100030ec             | Standard library function          |

##### IEEE 754 Decoding of 0x4045400000000000

```
r3 = 0x40454000 = 0100 0000 0100 0101 0100 0000 0000 0000
r2 = 0x00000000 = 0000 0000 0000 0000 0000 0000 0000 0000

Sign bit (bit 63):    0 → Positive
Exponent (bits 62-52): 10000000100 = 1028 → 1028 - 1023 = 5
Mantissa (bits 51-0):  0101010000...0 → 1.010101 (with implied 1)

Value = 1.010101₂ × 2⁵ = 101010.1₂ = 32 + 8 + 2 + 0.5 = 42.5 ✓
```

##### Decompiled main() After Renaming

```c
int main(void)
{
    stdio_init_all();
    do {
        __wrap_printf("fav_num: %f\r\n", /* r2:r3 = 0x4045400000000000 = 42.5 */);
    } while (true);
}
```

#### Reflection Answers

1. **Why does the compiler promote a `float` to a `double` when passing it to `printf`?**
   The C standard (§6.5.2.2) specifies **default argument promotions** for variadic functions like `printf`. When a `float` is passed to a variadic parameter (the `...` part), it is automatically promoted to `double`. This is because historically, floating-point hardware and calling conventions operated more efficiently with double precision. The `printf` function with `%f` always expects a 64-bit `double` on the stack or in the register pair `r2:r3`, never a 32-bit `float`.

2. **The low word (`r2`) is `0x00000000`. What does this tell you about the fractional part of `42.5`?**
   It means the fractional part of 42.5 can be represented exactly with very few mantissa bits. The value 0.5 is exactly 2⁻¹ in binary—a single bit. After normalization, the mantissa is `010101000...` which only needs 6 significant bits. All remaining 46 bits (including the entire low 32-bit word) are zero. Values like 0.5, 0.25, 0.125 (negative powers of 2) and their sums always produce clean low words, while values like 0.1 or 0.3 produce repeating binary fractions that fill both words.

3. **What is the purpose of the exponent bias (1023) in IEEE 754 double-precision?**
   The bias allows the exponent field to represent both positive and negative exponents using only unsigned integers. The 11-bit exponent field stores values 0–2047. By subtracting the bias (1023), the actual exponent range is −1022 to +1023. This avoids needing a separate sign bit for the exponent and simplifies hardware comparison—doubles can be compared as unsigned integers (for positive values) because larger exponents produce larger bit patterns. The bias value 1023 = 2¹⁰ − 1 is chosen to center the range symmetrically.

4. **If the sign bit (bit 63) were `1` instead of `0`, what value would the double represent?**
   The value would be **−42.5**. The sign bit in IEEE 754 is independent of all other fields: flipping bit 63 from 0 to 1 simply negates the value. The hex encoding would change from `0x4045400000000000` to `0xC045400000000000`—only the most significant nibble changes from `4` (`0100`) to `C` (`1100`), with bit 31 of r3 changing from 0 to 1.

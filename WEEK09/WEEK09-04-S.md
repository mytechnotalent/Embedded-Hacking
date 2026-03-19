# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 9
Operators in Embedded Systems: Debugging and Hacking Operators w/ DHT11 Basics

### Non-Credit Practice Exercise 4 Solution: Find All printf Format Strings

#### Answers

##### Complete String Catalog

| # | String                                              | Type           | Used By      |
|---|-----------------------------------------------------|---------------|-------------|
| 1 | `"arithmetic_operator: %d\r\n"`                     | Format string  | printf #1   |
| 2 | `"increment_operator: %d\r\n"`                      | Format string  | printf #2   |
| 3 | `"relational_operator: %d\r\n"`                     | Format string  | printf #3   |
| 4 | `"logical_operator: %d\r\n"`                        | Format string  | printf #4   |
| 5 | `"bitwise_operator: %d\r\n"`                        | Format string  | printf #5   |
| 6 | `"assignment_operator: %d\r\n"`                     | Format string  | printf #6   |
| 7 | `"Humidity: %.1f%%, Temperature: %.1f°C\r\n"`       | Format string  | printf #7   |
| 8 | `"DHT11 read failed\r\n"`                           | Plain string   | puts         |

##### Format Specifier Analysis

| Specifier | Meaning                                   | Used In        |
|-----------|------------------------------------------|---------------|
| `%d`      | Signed decimal integer                    | Strings 1–6   |
| `%.1f`    | Float with 1 decimal place               | String 7       |
| `%%`      | Literal percent sign (escaped)           | String 7       |
| `\r\n`    | Carriage return + line feed (0x0D 0x0A)  | All strings   |
| `°C`      | UTF-8 degree symbol + C (0xC2 0xB0 0x43)| String 7       |

##### Expected Operator Output Values

| Operator               | Expression             | Value | Explanation                        |
|-----------------------|------------------------|-------|------------------------------------|
| arithmetic_operator    | 5 × 10                | 50    | Multiplication                     |
| increment_operator     | x++ (x=5)             | 5     | Post-increment returns old value   |
| relational_operator    | 6 > 10                | 0     | False                              |
| logical_operator       | (6>10) && (10>6)      | 0     | Short-circuit: first operand false |
| bitwise_operator       | 6 << 1                | 12    | Left shift = multiply by 2        |
| assignment_operator    | 6 + 5                 | 11    | Addition assignment                |

##### GDB Search Commands

```gdb
(gdb) x/20s 0x10003e00
(gdb) x/50s 0x10003d00
```

##### Special Byte Sequences in Strings

| Sequence | Bytes       | Meaning              |
|----------|------------|---------------------|
| `\r\n`   | 0x0D 0x0A  | CRLF line ending    |
| `%%`     | 0x25 0x25  | Literal % character |
| `°`      | 0xC2 0xB0  | UTF-8 degree symbol |

#### Reflection Answers

1. **The humidity/temperature format string contains %%. What would happen if you patched one of the % characters to a different character (e.g., changed %% to %,)?**
   The `%%` escape produces a literal `%` in the output. If you change it to `%,` (bytes `25 2C`), `printf` would interpret `%,` as the start of a format specifier where `,` is the conversion character. Since `,` is not a valid `printf` conversion specifier, the behavior is **undefined** — most implementations would either print garbage, skip it, or consume the next argument from the stack incorrectly. This could corrupt the remaining output or even crash if `printf` tries to read a non-existent argument.

2. **If you changed "arithmetic_operator" to "hacked_operator__" (same length) in the binary, what would the serial output look like? Would the computed value change?**
   The serial output would show `hacked_operator__: 50` instead of `arithmetic_operator: 50`. The **computed value (50) would not change** — it's determined by the actual multiplication instruction in the code, not by the format string. The format string is just a label for display purposes. The `%d` specifier still reads the same `r1` register value (50) passed as the second argument to `printf`.

3. **What happens if you make a format string 1 byte longer (e.g., add a character)? Where would the extra byte be stored?**
   The extra byte would overwrite the **null terminator** (`0x00`) of the current string, and the byte after that is the first byte of the next consecutive string in `.rodata`. This effectively merges the two strings: `printf` would continue reading past the intended end into the next string's data until it finds another `0x00`. The output would include garbage characters from the adjacent string. If the adjacent data happens to contain `%` followed by a valid specifier, `printf` might try to read additional arguments from the stack, potentially causing a crash or information leak.

4. **The "DHT11 read failed" message uses puts instead of printf. Why would the compiler choose puts over printf for this particular string?**
   The compiler (with optimizations enabled) recognizes that `printf("DHT11 read failed\r\n")` has **no format specifiers** — it's a plain string with no `%d`, `%s`, `%f`, etc. Since no formatting is needed, the compiler optimizes it to `puts("DHT11 read failed")` (which automatically appends a newline). This is a common GCC optimization (`-fprintf-return-value`) because `puts` is simpler and faster than `printf` — it doesn't need to parse the format string looking for specifiers. The `\r\n` may be handled slightly differently depending on the implementation, but the key insight is that the compiler automatically selects the more efficient function.

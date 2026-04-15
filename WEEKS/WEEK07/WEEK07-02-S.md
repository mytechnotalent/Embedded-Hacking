# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 7
Constants in Embedded Systems: Debugging and Hacking Constants w/ 1602 LCD I2C Basics

### Non-Credit Practice Exercise 2 Solution: Find All String Literals in the Binary

#### Answers

##### String Catalog

| Address        | File Offset | String Content           | Length | Purpose                     |
|---------------|-------------|--------------------------|--------|-----------------------------|
| `0x10003ee8`  | `0x3EE8`   | "Reverse"                | 8      | LCD line 1 text             |
| `0x10003ef0`  | `0x3EF0`   | "Engineering"            | 12     | LCD line 2 text             |
| `0x10003efc`  | `0x3EFC`   | "FAV_NUM: %d\r\n"        | 16     | printf format string        |
| `0x10003f0c`  | `0x3F0C`   | "OTHER_FAV_NUM: %d\r\n"  | 22     | printf format string        |
| Various       | Various     | SDK panic/assert strings | Varies | Pico SDK internal messages  |
| Various       | Various     | Source file paths         | Varies | SDK debug/assert references |

##### GDB String Search Commands

```gdb
(gdb) x/s 0x10003ee8
0x10003ee8: "Reverse"

(gdb) x/s 0x10003ef0
0x10003ef0: "Engineering"

(gdb) x/s 0x10003efc
0x10003efc: "FAV_NUM: %d\r\n"

(gdb) x/s 0x10003f0c
0x10003f0c: "OTHER_FAV_NUM: %d\r\n"
```

##### Scanning for Strings

```gdb
(gdb) x/20s 0x10003e00
(gdb) x/50s 0x10003d00
```

##### Literal Pool Reference

From the literal pool at `0x100002a4`:

| Pool Address    | Value          | String It Points To       |
|----------------|---------------|---------------------------|
| `0x100002ac`   | `0x10003EE8`  | "Reverse"                 |
| `0x100002b0`   | `0x10003EF0`  | "Engineering"             |
| `0x100002b4`   | `0x10003EFC`  | "FAV_NUM: %d\r\n"         |
| `0x100002b8`   | `0x10003F0C`  | "OTHER_FAV_NUM: %d\r\n"   |

#### Reflection Answers

1. **How many distinct strings did you find? Were any of them surprising or unexpected?**
   At minimum 4 application-level strings: "Reverse", "Engineering", "FAV_NUM: %d\r\n", and "OTHER_FAV_NUM: %d\r\n". Beyond these, the Pico SDK embeds additional strings — panic handler messages, assert failure messages, and source file path strings used for debug output. The SDK strings are surprising because they reveal internal implementation details: file paths expose the build environment directory structure, and error messages reveal which SDK functions have built-in error checking. A reverse engineer can learn the SDK version and build configuration just from these strings.

2. **Why are strings so valuable to a reverse engineer? What can an attacker learn about a program just from its strings?**
   Strings are high-entropy human-readable data that reveals program behavior without reading assembly. An attacker can learn: what the program displays or communicates (LCD messages, serial output), what libraries it uses (SDK error messages), how it handles errors (panic/assert strings), what data formats it processes (`printf` format strings with `%d`, `%s`, `%f`), network endpoints or credentials (URLs, passwords, API keys), the build environment (file paths), and the overall purpose of the firmware. Strings are often the first thing a reverse engineer examines in an unknown binary.

3. **What technique could a developer use to make strings harder to find in a binary? (Think about what the strings look like in memory.)**
   String encryption/obfuscation: encrypt all string literals at compile time using XOR, AES, or a custom cipher, and decrypt them into a RAM buffer only when needed at runtime. This way, scanning the binary with `strings` or a hex editor reveals only ciphertext — random-looking bytes instead of readable text. Other techniques include: splitting strings across multiple locations and assembling them at runtime, using character arrays initialized by code rather than string literals, replacing strings with numeric lookup indices into an encrypted table, or using compile-time obfuscation tools that automatically transform string constants.

4. **The printf format strings contain \r\n. In the binary, these appear as two bytes: 0x0D 0x0A. Why two bytes instead of the four characters \, r, \, n?**
   The C compiler processes escape sequences during compilation. In source code, `\r` is written as two characters (backslash + r), but the compiler converts it to a single byte: `0x0D` (carriage return, ASCII 13). Similarly, `\n` becomes `0x0A` (line feed, ASCII 10). These are **control characters** — non-printable ASCII codes that control terminal behavior. The backslash notation is just a human-readable way to represent these bytes in source code. By the time the string reaches the binary, all escape sequences have been resolved to their single-byte equivalents.

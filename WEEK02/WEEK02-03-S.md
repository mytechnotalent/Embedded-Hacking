# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 2
Hello, World - Debugging and Hacking Basics: Debugging and Hacking a Basic Program for the Pico 2

### Non-Credit Practice Exercise 3 Solution: Examine Memory Around Your String

#### Answers

##### GDB Commands

```gdb
(gdb) set {char[14]} 0x20000000 = {'h','a','c','k','y',',',' ','w','o','r','l','d','\r','\0'}
(gdb) x/20b 0x20000000
```

##### Byte Dump Output

```
0x20000000: 0x68 0x61 0x63 0x6b 0x79 0x2c 0x20 0x77
0x20000008: 0x6f 0x72 0x6c 0x64 0x0d 0x00 0x00 0x00
0x20000010: 0x00 0x00 0x00 0x00
```

##### ASCII Mapping

| Offset | Hex Value | Character |
|--------|-----------|-----------|
| 0x00 | `0x68` | h |
| 0x01 | `0x61` | a |
| 0x02 | `0x63` | c |
| 0x03 | `0x6b` | k |
| 0x04 | `0x79` | y |
| 0x05 | `0x2c` | , (comma) |
| 0x06 | `0x20` | (space) |
| 0x07 | `0x77` | w |
| 0x08 | `0x6f` | o |
| 0x09 | `0x72` | r |
| 0x0a | `0x6c` | l |
| 0x0b | `0x64` | d |
| 0x0c | `0x0d` | \r (carriage return) |
| 0x0d | `0x00` | \0 (null terminator) |

#### Reflection Answers

1. **Which bytes mark the end of the printable string, and why are they needed?**
   The last two meaningful bytes are `0x0d` (carriage return `\r`) and `0x00` (null terminator `\0`). The null terminator signals the end of the string to `puts()`. Without it, `puts()` would read past the intended string and print garbage memory until a null byte is encountered.

2. **How would misaligned writes show up in the byte view?**
   If you write to an incorrect address or use wrong character offsets, the byte dump would show unexpected values at wrong positions. Characters would appear shifted, and adjacent data structures could be corrupted.

3. **What risks arise if you overwrite bytes immediately after your string?**
   Overwriting adjacent bytes could corrupt other data structures in SRAM, such as variables, linked lists, or runtime metadata. This could cause unpredictable crashes or silent data corruption depending on what occupies those memory locations.

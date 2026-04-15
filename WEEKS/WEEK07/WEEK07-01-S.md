# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 7
Constants in Embedded Systems: Debugging and Hacking Constants w/ 1602 LCD I2C Basics

### Non-Credit Practice Exercise 1 Solution: Change Both LCD Lines

#### Answers

##### String Locations in Flash

| String         | Address       | File Offset | Length (bytes) | Hex Encoding                                      |
|---------------|--------------|-------------|----------------|---------------------------------------------------|
| "Reverse"      | 0x10003ee8   | 0x3EE8      | 8 (7 + null)   | 52 65 76 65 72 73 65 00                            |
| "Engineering"  | 0x10003ef0   | 0x3EF0      | 12 (11 + null) | 45 6E 67 69 6E 65 65 72 69 6E 67 00               |

##### Line 1 Patch: "Reverse" → "Exploit"

| Character | Hex    |
|-----------|--------|
| E         | 0x45   |
| x         | 0x78   |
| p         | 0x70   |
| l         | 0x6c   |
| o         | 0x6f   |
| i         | 0x69   |
| t         | 0x74   |
| \0        | 0x00   |

```
Offset 0x3EE8:
Before: 52 65 76 65 72 73 65 00  ("Reverse")
After:  45 78 70 6C 6F 69 74 00  ("Exploit")
```

##### Line 2 Patch: "Engineering" → "Hacking!!!!"

| Character | Hex    |
|-----------|--------|
| H         | 0x48   |
| a         | 0x61   |
| c         | 0x63   |
| k         | 0x6b   |
| i         | 0x69   |
| n         | 0x6e   |
| g         | 0x67   |
| !         | 0x21   |
| !         | 0x21   |
| !         | 0x21   |
| !         | 0x21   |
| \0        | 0x00   |

```
Offset 0x3EF0:
Before: 45 6E 67 69 6E 65 65 72 69 6E 67 00  ("Engineering")
After:  48 61 63 6B 69 6E 67 21 21 21 21 00  ("Hacking!!!!")
```

##### Conversion and Flash

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0017_constants
python ..\uf2conv.py build\0x0017_constants-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

##### LCD Verification

```
Line 1: Exploit
Line 2: Hacking!!!!
```

#### Reflection Answers

1. **Why must the replacement string be the same length (or shorter) as the original? What specific data would you corrupt if you used a longer string?**
   Strings are stored consecutively in the `.rodata` section. "Reverse" occupies 8 bytes starting at `0x10003ee8` and "Engineering" starts immediately at `0x10003ef0`. If the replacement string is longer than 8 bytes, the extra bytes would overwrite the beginning of "Engineering" (or whatever data follows). The `.rodata` section has no gaps—it's a packed sequence of constants, format strings, and other read-only data. Corrupting adjacent data could break LCD line 2, crash `printf` format strings, or cause undefined behavior.

2. **The two strings are stored only 8 bytes apart (0x3EE8 to 0x3EF0). "Reverse" is 7 characters + null = 8 bytes. What would happen if you patched "Reverse" with "Reversal" (8 characters + null = 9 bytes)?**
   "Reversal" needs 9 bytes (8 chars + null terminator). The 9th byte (the `0x00` null terminator) would be written to address `0x10003ef0`, which is the first byte of "Engineering" — the letter 'E' (`0x45`). This would overwrite 'E' with `0x00`, turning "Engineering" into an empty string. The LCD would display "Reversal" on line 1 and nothing on line 2, because `lcd_puts` would see a null terminator immediately at the start of the second string.

3. **If you wanted the LCD to display "Hello" on line 1 (5 characters instead of 7), what would you put in the remaining 2 bytes plus null? Write out the full 8-byte hex sequence.**
   "Hello" = 5 characters, followed by the null terminator and 2 padding null bytes:
   ```
   48 65 6C 6C 6F 00 00 00
   H  e  l  l  o  \0 \0 \0
   ```
   The first `0x00` at position 5 terminates the string. The remaining two `0x00` bytes are padding that fills the original 8-byte allocation. These padding bytes are never read by `lcd_puts` because it stops at the first null terminator.

4. **Could you change the LCD to display nothing on line 1 by patching just one byte? Which byte and what value?**
   Yes. Change the first byte at offset `0x3EE8` from `0x52` ('R') to `0x00` (null). This makes the string start with a null terminator, so `lcd_puts` sees an empty string and displays nothing. Only one byte needs to change: the byte at file offset `0x3EE8`, from `0x52` to `0x00`.

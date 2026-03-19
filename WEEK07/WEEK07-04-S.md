# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 7
Constants in Embedded Systems: Debugging and Hacking Constants w/ 1602 LCD I2C Basics

### Non-Credit Practice Exercise 4 Solution: Display Your Own Custom Message on the LCD

#### Answers

##### String Constraints

| Line | Original      | Address      | File Offset | Max Chars | Allocated Bytes |
|------|--------------|-------------|-------------|-----------|-----------------|
| 1    | "Reverse"     | 0x10003ee8  | 0x3EE8      | 7         | 8               |
| 2    | "Engineering" | 0x10003ef0  | 0x3EF0      | 11        | 12              |

##### Example Patch: "Hello!!" and "World!!"

**Line 1: "Hello!!" (7 characters — exact fit)**

| Character | Hex    |
|-----------|--------|
| H         | 0x48   |
| e         | 0x65   |
| l         | 0x6C   |
| l         | 0x6C   |
| o         | 0x6F   |
| !         | 0x21   |
| !         | 0x21   |
| \0        | 0x00   |

```
Offset 0x3EE8:
Before: 52 65 76 65 72 73 65 00  ("Reverse")
After:  48 65 6C 6C 6F 21 21 00  ("Hello!!")
```

**Line 2: "World!!" (7 characters — needs 5 bytes of null padding)**

| Character | Hex    |
|-----------|--------|
| W         | 0x57   |
| o         | 0x6F   |
| r         | 0x72   |
| l         | 0x6C   |
| d         | 0x64   |
| !         | 0x21   |
| !         | 0x21   |
| \0        | 0x00   |
| \0 (pad)  | 0x00   |
| \0 (pad)  | 0x00   |
| \0 (pad)  | 0x00   |
| \0 (pad)  | 0x00   |

```
Offset 0x3EF0:
Before: 45 6E 67 69 6E 65 65 72 69 6E 67 00  ("Engineering")
After:  57 6F 72 6C 64 21 21 00 00 00 00 00  ("World!!")
```

##### Example Patch: Short String "Hi"

**Line 1: "Hi" (2 characters — needs 5 bytes of null padding)**

```
Offset 0x3EE8:
Before: 52 65 76 65 72 73 65 00  ("Reverse")
After:  48 69 00 00 00 00 00 00  ("Hi")
```

##### Conversion and Flash

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0017_constants
python ..\uf2conv.py build\0x0017_constants-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

#### Reflection Answers

1. **You padded short strings with 0x00 null bytes. Would it also work to pad with 0x20 (space characters)? What would be the difference on the LCD display?**
   Both approaches produce valid strings, but the display differs. With `0x00` padding, the string terminates at the first null byte — `lcd_puts` stops reading there, and the remaining bytes are ignored. The LCD shows only your text. With `0x20` (space) padding, the spaces become part of the string — `lcd_puts` sends them to the LCD as visible blank characters. The LCD would show your text followed by trailing spaces. Functionally both work, but `0x00` padding is cleaner because the string length matches your intended text, and the LCD positions after your text remain in whatever state the LCD controller initialized them to (typically blank anyway).

2. **The LCD is a 1602 (16 columns × 2 rows). What would happen if you could somehow put a 20-character string in memory? Would the LCD display all 20, or only the first 16?**
   The LCD would display only the first 16 characters in the visible area. The HD44780 controller (used in 1602 LCD modules) has 40 bytes of DDRAM per line, so characters 17-20 would be written to DDRAM but are beyond the visible 16-column window. They would only become visible if you issued a display shift command to scroll the view. The `lcd_puts` function writes all characters to the controller regardless of line length — it has no built-in truncation. The 16-character limit is a physical display constraint, not a software one.

3. **If you wanted to combine the string hacks from Exercise 1 (changing both LCD lines) AND a hypothetical numeric hack (e.g., changing the movs r1, #42 encoding at offset 0x28E), could you do all patches in a single .bin file? What offsets would you need to modify?**
   Yes, all patches can be applied to the same `.bin` file since they are at non-overlapping offsets. The three patch locations are:
   - **Offset 0x28E**: FAV_NUM — change `movs r1, #42` immediate byte from `0x2A` to desired value (1 byte)
   - **Offset 0x3EE8**: LCD line 1 — replace the 8-byte "Reverse" string
   - **Offset 0x3EF0**: LCD line 2 — replace the 12-byte "Engineering" string
   
   Each patch modifies a different region of the binary, so they are completely independent. You could also patch the `movw r1, #1337` instruction at offset `0x298` (the imm8 byte of the OTHER_FAV_NUM encoding) for a fourth independent patch.

4. **Besides LCD text, what other strings could you patch in a real-world embedded device to change its behavior? Think about Wi-Fi SSIDs, Bluetooth device names, HTTP headers, etc.**
   Real-world embedded devices contain many patchable strings: **Wi-Fi SSIDs** and **passwords** (change what network the device connects to), **Bluetooth device names** (change how it appears during pairing), **HTTP/HTTPS URLs** (redirect API calls to a different server), **MQTT broker addresses** (redirect IoT telemetry), **DNS hostnames**, **firmware version strings** (spoof version for update bypass), **serial number formats**, **command-line interface prompts**, **error and debug messages** (hide forensic evidence), **TLS/SSL certificate fields**, **NTP server addresses** (manipulate time synchronization), and **authentication tokens or API keys**. String patching is one of the most practical firmware modification techniques because it's simple to execute and can dramatically change device behavior.

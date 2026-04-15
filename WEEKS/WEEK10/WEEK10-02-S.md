# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 10
Dynamic Conditionals in Embedded Systems: Debugging and Hacking Dynamic Conditionals w/ SG90 Servo Basics

### Non-Credit Practice Exercise 2 Solution: Add a Third Command

#### Answers

##### Patch 1: Compare Byte '2' → '3'

Change `cmp r4, #0x32` to `cmp r4, #0x33`:

```
Before: 32 2C  (cmp r4, #0x32 = '2')
After:  33 2C  (cmp r4, #0x33 = '3')
```

Only the immediate byte changes: `0x32` → `0x33`.

##### Patch 2: Case Angle 1 — 180.0f → 90.0f

```
Before: 00 00 34 43  (180.0f)
After:  00 00 B4 42  (90.0f)
```

##### Patch 3: Case Angle 2 — 0.0f → 90.0f

```
Before: 00 00 00 00  (0.0f)
After:  00 00 B4 42  (90.0f)
```

##### IEEE-754 Reference

| Angle  | Hex          | Little-Endian   |
|--------|-------------|----------------|
| 0.0f   | 0x00000000  | 00 00 00 00    |
| 90.0f  | 0x42B40000  | 00 00 B4 42    |
| 180.0f | 0x43340000  | 00 00 34 43    |

##### Behavior After Patch

| Key | Action                               |
|-----|--------------------------------------|
| '1' | Sweep 0° → 180° (unchanged)         |
| '3' | Move to 90° center (new command)     |
| '2' | Falls to default — prints "??"       |

#### Reflection Answers

1. **Why does this exercise repurpose the existing case '2' path instead of adding a completely new branch? What would adding a new branch require?**
   Adding a new branch would require inserting new instructions into the binary — additional `cmp`, `beq`, angle-loading code, and a `servo_set_angle` call. This would shift all subsequent code addresses, breaking every PC-relative branch, literal pool reference, and function call in the program. In a compiled binary without relocation information, inserting bytes is extremely difficult. Repurposing the existing case '2' path reuses the existing branch structure, angle-loading instructions, and function calls — only the data values change, not the code layout.

2. **The cmp instruction uses an 8-bit immediate field. What is the range of characters you could compare against? Could you use a non-ASCII value?**
   The `cmp Rn, #imm8` Thumb instruction has an 8-bit unsigned immediate, giving a range of 0–255 (`0x00`–`0xFF`). This covers all ASCII characters (0–127) plus extended values (128–255). You could compare against any byte value, including non-printable characters (`0x01`–`0x1F`), DEL (`0x7F`), or extended characters (`0x80`–`0xFF`). However, the user needs to be able to type the character via `getchar()` — non-printable characters would require special terminal input (e.g., Ctrl combinations).

3. **How would you keep BOTH the original '2' command AND add '3' as a new command, using only data patches (no instruction insertion)?**
   You could repurpose the **default/else** branch path. After the `cmp r4, #0x32` (case '2'), there's typically a branch to a default handler that prints "??". If you change the compare in the default path (or an unused branch) to `cmp r4, #0x33`, and redirect its logic to reuse one of the existing `servo_set_angle` code paths, you could handle both. Alternatively, if the binary has any unreachable code or NOP sleds, you could repurpose that space. The constraint is that you cannot increase the binary size — only modify existing bytes.

4. **What would happen if you changed the compare value to 0x00 (null)? Could a user ever trigger this case?**
   A compare against `0x00` would trigger on a null byte. In terminal input via `getchar()`, a null character is not easily typed — most terminals don't send `0x00` on any key press. On some systems, Ctrl+@ or Ctrl+Shift+2 generates a null byte, but this is platform-dependent. In practice, comparing against `0x00` would create an unreachable case — the command would exist in the binary but could never be triggered via normal serial terminal input, effectively making it a dead code path.

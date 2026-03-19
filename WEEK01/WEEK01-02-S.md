# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Non-Credit Practice Exercise 2 Solution: Find Strings in Ghidra

#### Answers

##### String Location

| Item | Answer |
|------|--------|
| **String Address** | `0x100019CC` |
| **Actual String Content** | `"hello, world\r\n"` |
| **String Length** | 14 bytes |
| **Located In** | Flash memory (XIP region, starts with `0x10000...`) |

##### Question 1: What is the address and is it Flash or RAM?
The string `"hello, world\r\n"` is located at address **`0x100019CC`** in **Flash memory**. We know it is Flash because the address begins with `0x10000...` (the XIP/Execute-In-Place region starts at `0x10000000`). RAM addresses start at `0x20000000`.

##### Question 2: How many bytes does the string take?
**14 bytes** total:
- 12 printable characters: `h`, `e`, `l`, `l`, `o`, `,`, ` `, `w`, `o`, `r`, `l`, `d`
- 2 special characters: `\r` (carriage return, 0x0D) and `\n` (newline, 0x0A)

##### Question 3: How many times and which functions reference it?
The string is referenced **1 time**, only in the **`main()`** function. The `ldr` instruction at `0x1000023a` loads the string address into register `r0`, which is then passed to `__wrap_puts`.

##### Question 4: How is the string encoded?
The string is encoded in **ASCII**. Each character occupies exactly one byte:
- `\r` = `0x0D` (carriage return)
- `\n` = `0x0A` (newline/line feed)

##### Expected Output

```
String Found: "hello, world\r\n"
Address: 0x100019CC
Located in: Flash (XIP region)
Total Size: 14 bytes
Referenced by: main()
Used in: printf() argument (optimized to __wrap_puts) to print the string in an infinite loop
```

#### Reflection Answers

1. **Why is the string stored in Flash instead of RAM?**
   String literals are constants that never change. Storing them in Flash (read-only) saves precious RAM for variables and the stack. The XIP feature allows the processor to read directly from Flash.

2. **What would happen if you tried to modify this string at runtime?**
   Flash memory is read-only at runtime. Attempting to write to a Flash address would cause a fault. To modify printed output, you must write your new string to SRAM (`0x20000000+`) and redirect the pointer.

3. **How does the Listing view help you understand string storage?**
   The Listing view shows the raw hex bytes alongside their ASCII interpretation, letting you see exactly how each character maps to memory and where the string boundaries are.

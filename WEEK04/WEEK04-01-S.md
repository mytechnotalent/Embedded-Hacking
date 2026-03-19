# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 4
Variables in Embedded Systems: Debugging and Hacking Variables w/ GPIO Output Basics

### Non-Credit Practice Exercise 1 Solution: Analyze Variable Storage in Ghidra

#### Answers

##### Main Function Analysis

| Item                  | Value/Location | Notes                           |
|-----------------------|---------------|---------------------------------|
| Main function address | 0x10000234    | Entry point of program          |
| Age value (hex)       | 0x2b          | Optimized constant              |
| Age value (decimal)   | 43            | Original variable value         |
| Variable in memory?   | No            | Compiler optimized it away      |
| printf address        | 0x10003100    | Standard library function       |
| stdio_init_all addr   | 0x100030cc    | I/O initialization              |
| Format string         | "age: %d\r\n" | Located in .rodata section      |

##### Decompiled main() After Renaming

```c
int main(void)
{
    stdio_init_all();
    do {
        printf("age: %d\r\n", 0x2b);
    } while (true);
}
```

##### Hex-to-Decimal Conversion

```
0x2b = (2 × 16) + 11 = 32 + 11 = 43
```

The compiler replaced both `age = 42` and `age = 43` with the final constant value `0x2b` (43) as an immediate operand in `movs r1, #0x2b`.

##### Assembly Listing

```assembly
movs    r1, #0x2b       ; Load 43 directly into r1 (printf argument)
ldr     r0, [pc, #...]  ; Load format string address
bl      printf           ; Call printf
```

#### Reflection Answers

1. **Why did the compiler optimize away the `age` variable?**
   The compiler performs **dead store elimination** and **constant propagation**. The initial assignment `age = 42` is immediately overwritten by `age = 43` with no intervening reads of the value 42. Since the only value ever observed is 43, the compiler replaces all references to `age` with the constant `0x2b` (43) as an immediate operand. No memory allocation is needed—the value lives entirely in the instruction encoding (`movs r1, #0x2b`).

2. **In what memory section would `age` have been stored if it wasn't optimized away?**
   As a local variable declared inside `main()`, `age` would have been stored on the **stack** (in RAM at `0x2000xxxx`). The compiler would allocate space by subtracting from SP, store the value with a `str` instruction, and load it back with `ldr` before passing it to `printf`. If `age` were declared as a global initialized variable, it would be placed in the **`.data`** section (RAM, initialized from flash at boot). If declared as `static` or global but uninitialized, it would go in the **`.bss`** section (RAM, zeroed at boot).

3. **Where is the string "age: %d\r\n" stored, and why can't it be in RAM?**
   The format string is stored in the **`.rodata`** (read-only data) section in flash memory at `0x1000xxxx`. It cannot be in RAM because: (a) string literals are constants that never change, so storing them in limited RAM would waste space; (b) flash is non-volatile—the string persists across power cycles without needing to be copied from anywhere; (c) the XIP (Execute In Place) mechanism allows the CPU to read directly from flash, so `.rodata` access is efficient.

4. **What would happen if we had used `age` in a calculation before reassigning it to 43?**
   The compiler would be forced to preserve the value 42 because it's actually read before being overwritten. For example, `printf("before: %d\n", age); age = 43;` would require the compiler to generate both `movs r1, #0x2a` (42) for the first print and `movs r1, #0x2b` (43) for subsequent uses. Alternatively, a calculation like `age = age + 1` would allow the compiler to constant-fold `42 + 1 = 43` at compile time and still emit just `#0x2b`. Only if the value depends on runtime input would the variable require actual memory allocation.

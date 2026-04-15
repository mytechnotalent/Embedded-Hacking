# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Non-Credit Practice Exercise 3 Solution: Find Cross-References in Ghidra

#### Answers

##### Fill-in-the-Blank Items

| Item | Answer |
|------|--------|
| **Data reference address** | `0x10000244` (DAT_10000244) |
| **Number of references** | 1 |
| **Reference type** | Read (`ldr` instruction) |
| **Function using it** | `main()` |
| **Next instruction after ldr** | `bl __wrap_puts` at `0x100015fc` |

##### Question 1: What is the address of the data reference?
The data reference is at **`0x10000244`** (labeled `DAT_10000244` in Ghidra). This location stores the pointer value `0x100019CC`, which is the address of the `"hello, world"` string.

##### Question 2: How many places reference this data?
**1 place** - it is only referenced in the `main()` function via the `ldr` instruction.

##### Question 3: Is it a read or write operation? Why?
It is a **READ** operation. The `ldr` (Load Register) instruction reads the pointer value from `DAT_10000244` into register `r0`. The program needs to read this pointer to pass the string address as an argument to `__wrap_puts`.

##### Question 4: What happens next?
After the `ldr r0, [DAT_10000244]` instruction loads the string address into `r0`, the next instruction is **`bl 0x100015fc <__wrap_puts>`** which calls the `puts` function with `r0` as its argument (the string pointer).

##### Question 5: Complete Data Flow Chain

```
String "hello, world\r\n" stored at 0x100019CC (Flash)
    |
    v
Pointer to string stored at DAT_10000244 (0x10000244)
    |
    v
main() executes: ldr r0, [DAT_10000244]  -> r0 = 0x100019CC
    |
    v
main() executes: bl __wrap_puts           -> prints the string
    |
    v
main() executes: b.n main+6              -> loops back (infinite loop)
```

#### Reflection Answers

1. **Why does the compiler use a pointer (indirect reference) instead of embedding the string address directly in the instruction?**
   ARM Thumb instructions have limited immediate value sizes. The `ldr` instruction uses a PC-relative offset to reach a nearby literal pool entry (`DAT_10000244`) that holds the full 32-bit address. This pattern allows addressing any location in the 4 GB address space.

2. **What is a literal pool?**
   A literal pool is a region of constant data placed near the code that uses it. The compiler stores full 32-bit values here that cannot fit as immediates in Thumb instructions. The `ldr` instruction loads from the literal pool using a small PC-relative offset.

3. **How does cross-referencing help in reverse engineering?**
   Cross-references let you trace data flow through a program. Starting from a known string, you can find which functions use it, how data moves between functions, and understand the program's control flow without having source code.

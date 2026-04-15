# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 6
Static Variables in Embedded Systems: Debugging and Hacking Static Variables w/ GPIO Input Basics

### Non-Credit Practice Exercise 1 Solution: Change the Static Variable Initial Value from 42 to 100

#### Answers

##### Static Variable Location

| Item                       | Value        | Notes                           |
|---------------------------|-------------|--------------------------------|
| RAM address (runtime)     | 0x200005a8  | Where variable lives at runtime |
| Flash address (init value)| Calculated   | In .data section of flash       |
| Original value (hex)      | 0x2A        | 42 decimal                      |
| Patched value (hex)       | 0x64        | 100 decimal                     |
| File offset               | flash_addr - 0x10000000 | Binary base subtraction |

##### GDB Session

```gdb
(gdb) x/1db 0x200005a8
0x200005a8: 42
(gdb) find /b 0x10000000, 0x10010000, 0x2a
```

##### Serial Output After Patch

```
regular_fav_num: 42
static_fav_num: 100
regular_fav_num: 42
static_fav_num: 101
regular_fav_num: 42
static_fav_num: 102
...
```

#### Reflection Answers

1. **Why does the initial value live in flash AND get copied to RAM? Why not just use flash directly?**
   Static variables need to be **modifiable** at runtime—the program increments `static_fav_num` each iteration. Flash memory is read-only during normal execution (it requires a special erase/program sequence to modify). So the initial value is stored in flash as a template, and the startup code (`crt0.S`) copies the entire `.data` section from flash to RAM before `main()` runs. This gives the variable its correct starting value (42) in writable RAM where subsequent `adds` and `strb` instructions can modify it freely.

2. **The static variable wraps around at 255 (since it's `uint8_t`). After patching the initial value to 100, after how many iterations will it overflow back to 0?**
   A `uint8_t` overflows from 255 to 0. Starting at 100 and incrementing by 1: it takes `255 - 100 = 155` increments to reach 255, then one more to wrap to 0. So it overflows after **156 iterations**. Compare to the original: starting at 42, it takes `255 - 42 + 1 = 214` iterations.

3. **If you also wanted to change the `regular_fav_num` constant from 42, would you patch the same area of the binary? Why or why not?**
   No. `regular_fav_num` is a **local variable** that the compiler optimized to an immediate constant (`movs r1, #0x2a`), just like Week 4's `age` variable. It's encoded directly in the instruction opcode in the `.text` section, not in the `.data` section. You would need to find the `movs r1, #0x2a` instruction in the code and patch the immediate byte from `0x2a` to your desired value. The `.data` section only contains initialized static/global variables.

4. **What would happen if the `.data` section had TWO static variables — would their initial values be adjacent in flash?**
   Yes. The linker places all initialized static variables contiguously in the `.data` section. Their initial values are stored in the same order in flash, packed adjacent to each other (possibly with alignment padding). The startup code performs a single `memcpy`-like loop that copies the entire `.data` block from flash to RAM. So if you had `static uint8_t a = 42;` and `static uint8_t b = 99;`, the bytes `0x2A` and `0x63` would be adjacent (or nearly so) in flash, and both would be copied to their respective RAM addresses during boot.

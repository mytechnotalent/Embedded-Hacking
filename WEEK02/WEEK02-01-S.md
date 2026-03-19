# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 2
Hello, World - Debugging and Hacking Basics: Debugging and Hacking a Basic Program for the Pico 2

### Non-Credit Practice Exercise 1 Solution: Change the Message

#### Answers

##### Attack Summary
The goal is to write a custom message into SRAM at `0x20000000` and redirect `r0` to print it instead of the original `"hello, world"` string, without changing the source code.

##### GDB Commands

```gdb
(gdb) target extended-remote :3333
(gdb) monitor reset halt
(gdb) b *0x1000023c                    # Breakpoint before __wrap_puts
(gdb) c                                 # Continue to breakpoint
(gdb) set {char[20]} 0x20000000 = {'Y','o','u','r',' ','N','a','m','e','!','\r','\0'}
(gdb) set $r0 = 0x20000000             # Redirect r0 to injected message
(gdb) c                                 # Resume - serial shows custom message
```

##### Verification
```gdb
(gdb) x/s 0x20000000                   # Should show your injected message
(gdb) x/s 0x100019cc                   # Original string still in Flash
```

#### Reflection Answers

1. **Why does the string have to live in SRAM instead of flash during runtime?**
   Flash memory is read-only at runtime. The original string at `0x100019cc` cannot be modified. SRAM starting at `0x20000000` is read-write, so that is where we must place our replacement string.

2. **What would happen if you forgot the null terminator in your injected string?**
   `puts()` reads characters until it encounters `\0`. Without it, `puts()` would continue reading past the intended string, printing garbage characters from adjacent memory until a null byte happens to appear. This could crash the program or leak sensitive data.

3. **How does changing `r0` alter the behavior of `puts()` without touching source code?**
   In the ARM calling convention, the first function argument is passed in `r0`. When `bl __wrap_puts` executes at `0x1000023c`, it reads the string address from `r0`. By changing `r0` from `0x100019cc` (original Flash string) to `0x20000000` (our SRAM string), we redirect what `puts()` prints.

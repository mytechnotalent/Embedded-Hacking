# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Non-Credit Practice Exercise 1 Solution: Explore in Ghidra

#### Answers

##### Question 1: What does the function return?
`stdio_init_all()` returns `_Bool`. The function signature is `_Bool stdio_init_all(void)`.

##### Question 2: What parameters does it take?
None. The signature uses `(void)`, which means zero parameters.

##### Question 3: What functions does it call?
In this build, it calls `stdio_uart_init()` to initialize serial output.

##### Question 4: What's the purpose?
Its purpose is to initialize standard I/O so `printf()`/`puts()` output can be transmitted over UART.

##### Expected Output

```
stdio_init_all() returns: _Bool
It takes 0 parameters
It calls the following functions: UART init
Based on these calls, I believe it initializes: Standard I/O for UART serial communication
```

#### Reflection Answers

1. **Why would we need to initialize standard I/O before using `printf()`?**
   Without initialization, there is no configured output path. `printf()` needs a destination (UART in this exercise) to transmit characters.

2. **Can you find other functions in the Symbol Tree that might be related to I/O?**
   Yes - `stdio_uart_init`, `__wrap_puts`, and related low-level serial/output helpers are I/O-related.

3. **How does this function support the `printf("hello, world\r\n")` call in main?**
   It configures the UART output path so when `printf()` (optimized to `__wrap_puts`) runs, the string is sent over serial.

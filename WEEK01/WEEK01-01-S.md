# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 1
Introduction and Overview of Embedded Reverse Engineering: Ethics, Scoping, and Basic Concepts

### Non-Credit Practice Exercise 1 Solution: Explore in Ghidra

#### Answers

##### Question 1: What does the function return?
`stdio_init_all()` returns `_Bool`. The function signature shows `_Bool stdio_init_all(void)`.

##### Question 2: What parameters does it take?
**None** - the function signature shows `(void)` in parentheses, meaning zero parameters.

##### Question 3: What functions does it call?
`stdio_init_all()` calls initialization functions for:
- **UART** initialization (serial pin communication) `stdio_uart_init()`

These set up the standard I/O subsystem so that `printf()` can output data.

##### Question 4: What's the purpose?
`stdio_init_all()` initializes **Standard Input/Output** for the Pico 2:
- **std** = Standard
- **io** = Input/Output

It sets up UART communication channels, which allows `printf()` to send output through the serial connection.

##### Expected Output

```
stdio_init_all() returns: void (_Bool)
It takes 0 parameters
It calls the following functions: UART init
Based on these calls, I believe it initializes: Standard I/O for USB and UART serial communication
```

#### Reflection Answers

1. **Why would we need to initialize standard I/O before using `printf()`?**
   Without initialization, there is no communication channel configured. `printf()` needs a destination (USB, in other cases, or UART) to send its output. Without `stdio_init_all()`, output has nowhere to go.

2. **Can you find other functions in the Symbol Tree that might be related to I/O?**
   Yes - functions like `stdio_usb_init`, in other cases, and `stdio_uart_init`, `__wrap_puts`, and other I/O-related functions appear in the Symbol Tree.

3. **How does this function support the `printf("hello, world\r\n")` call in main?**
   It configures the USB, in other cases, and UART hardware so that when `printf()` (optimized to `__wrap_puts`) executes, the characters are transmitted over the serial connection to the host computer.

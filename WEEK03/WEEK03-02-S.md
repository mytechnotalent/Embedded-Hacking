# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 3
Embedded System Analysis: Understanding the RP2350 Architecture w/ Comprehensive Firmware Analysis

### Non-Credit Practice Exercise 2 Solution: Find the Stack Size

#### Answers

##### Initial Stack Pointer

```gdb
(gdb) x/x 0x10000000
0x10000000 <__vectors>: 0x20082000
```

The initial stack pointer is **0x20082000**, stored as the first entry in the vector table.

##### Stack Limit

The stack limit is **0x20078000**, defined by the linker script.

```gdb
(gdb) info symbol __StackLimit
__StackLimit in section .stack
```

##### Stack Size Calculation

| Value           | Hex          | Decimal       |
|-----------------|-------------|---------------|
| Stack Top       | 0x20082000  | 537,108,480   |
| Stack Limit     | 0x20078000  | 537,067,520   |
| **Stack Size**  | 0x0000A000  | **40,960 bytes** |

```
Stack Size = 0x20082000 - 0x20078000 = 0xA000 = 40,960 bytes
40,960 ÷ 1,024 = 40 KB
```

##### Memory Region Verification

| Region    | Start        | End          | Size   |
|-----------|-------------|-------------|--------|
| RAM       | 0x20000000  | 0x20080000  | 512 KB |
| SCRATCH_X | 0x20080000  | 0x20081000  | 4 KB   |
| SCRATCH_Y | 0x20081000  | 0x20082000  | 4 KB   |
| **Stack** | 0x20078000  | 0x20082000  | **40 KB** |

The stack spans from the upper portion of main RAM through SCRATCH_X and into SCRATCH_Y.

##### Runtime Stack Usage at main()

```gdb
(gdb) b main
(gdb) c
(gdb) info registers sp
sp             0x20081fc8       0x20081fc8
```

Stack used at `main()` entry: `0x20082000 - 0x20081fc8 = 0x38 = 56 bytes`

#### Reflection Answers

1. **Why is the stack 40 KB instead of just fitting in the 4 KB SCRATCH_Y region?**
   The stack pointer is initialized at the top of SCRATCH_Y (`0x20082000`), but the stack grows **downward**. As functions are called and local variables are allocated, the stack pointer decreases past the SCRATCH_Y boundary (`0x20081000`), through SCRATCH_X, and into the upper portion of main RAM. The linker sets the stack limit at `0x20078000` to give the stack 40 KB of room, which is sufficient for typical embedded applications with nested function calls.

2. **What happens if the stack grows beyond 0x20078000?**
   A **stack overflow** occurs. The stack would collide with the heap or global data stored in the lower portion of RAM. This can corrupt variables, overwrite heap metadata, or cause a HardFault if memory protection is enabled. On the RP2350 with Cortex-M33 MPU support, a MemManage fault could be triggered if stack guard regions are configured.

3. **How would you detect a stack overflow during runtime?**
   Several methods exist: (a) Write a known "canary" pattern (e.g., `0xDEADBEEF`) at the stack limit and periodically check if it's been overwritten. (b) Use the Cortex-M33 MPU to set a guard region at `0x20078000` that triggers a MemManage fault on access. (c) In GDB, use a watchpoint: `watch *(int*)0x20078000` to break if the stack reaches the limit. (d) Use the `stackusage` GDB macro from the exercise to monitor usage.

4. **Why does the stack grow downward instead of upward?**
   This is an ARM architecture convention inherited from early processor designs. Growing downward allows the stack and heap to grow toward each other from opposite ends of RAM, maximizing memory utilization without needing to know each region's size in advance. The stack starts at the highest address and grows down, while the heap starts at a lower address and grows up. If they meet, memory is exhausted—but this layout gives both regions the maximum possible space.

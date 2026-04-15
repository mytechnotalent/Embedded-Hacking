# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 7
Constants in Embedded Systems: Debugging and Hacking Constants w/ 1602 LCD I2C Basics

### Non-Credit Practice Exercise 3 Solution: Trace the I²C Struct Pointer Chain

#### Answers

##### Complete Pointer Chain

```
I2C_PORT                          (source macro: #define I2C_PORT i2c1)
    ↓
i2c1                              (SDK macro: #define i2c1 (&i2c1_inst))
    ↓
&i2c1_inst = 0x2000062c          (SRAM address of i2c_inst_t struct)
    ↓
i2c1_inst.hw = 0x40098000        (pointer to I²C1 hardware register base)
    ↓
I²C1 Hardware Registers           (memory-mapped I/O silicon)
    +-- IC_CON      at 0x40098000
    +-- IC_TAR      at 0x40098004
    +-- IC_SAR      at 0x40098008
    +-- IC_DATA_CMD at 0x40098010
```

##### Literal Pool Load

```gdb
(gdb) x/6wx 0x100002a4
0x100002a4: 0x000186a0  0x2000062c  0x10003ee8  0x10003ef0
0x100002b4: 0x10003efc  0x10003f0c
```

The value `0x2000062c` at pool address `0x100002a8` is loaded into `r0` by a `ldr r0, [pc, #offset]` instruction before the `bl i2c_init` call.

##### i2c1_inst Struct in SRAM

```gdb
(gdb) x/2wx 0x2000062c
0x2000062c <i2c1_inst>: 0x40098000      0x00000000
```

| Offset | Field             | Value        | Size    | Meaning                       |
|--------|-------------------|-------------|---------|-------------------------------|
| +0x00  | hw                | 0x40098000  | 4 bytes | Pointer to I²C1 hardware regs |
| +0x04  | restart_on_next   | 0x00000000  | 4 bytes | false (no pending restart)    |

Total struct size: 8 bytes (4-byte pointer + 4-byte bool padded to word alignment).

##### Hardware Registers at 0x40098000

```gdb
(gdb) x/8wx 0x40098000
```

| Offset | Register    | Address      | Description               |
|--------|-------------|-------------|---------------------------|
| +0x00  | IC_CON      | 0x40098000  | I²C control register      |
| +0x04  | IC_TAR      | 0x40098004  | Target address register   |
| +0x08  | IC_SAR      | 0x40098008  | Slave address register    |
| +0x10  | IC_DATA_CMD | 0x40098010  | Data command register     |

##### I²C0 Comparison

```gdb
(gdb) x/2wx 0x20000628
```

| Controller | Struct Address | hw Pointer   | Separation  |
|------------|---------------|-------------|-------------|
| I²C0       | 0x20000628    | 0x40090000  | Base        |
| I²C1       | 0x2000062c    | 0x40098000  | +0x8000     |

Same struct layout, different hardware pointer — demonstrating the SDK's abstraction.

#### Reflection Answers

1. **Why does the SDK use a struct with a pointer to hardware registers instead of accessing 0x40098000 directly? What advantage does this abstraction provide?**
   The struct abstraction allows the same code to work for both I²C controllers — I²C0 at `0x40090000` and I²C1 at `0x40098000` — by simply passing a different struct pointer. Functions like `i2c_init(i2c_inst_t *i2c, uint baudrate)` accept a pointer parameter, so one implementation serves both controllers. Without the struct, every I²C function would need either hardcoded addresses (duplicating code for each controller) or `if/else` branches. The abstraction also enables portability: if a future chip moves the hardware registers, only the struct initialization changes — not every function that accesses I²C.

2. **The hw pointer stores 0x40098000. In the binary, this appears as bytes 00 80 09 40. Why is the byte order reversed from how we write the address?**
   ARM Cortex-M33 uses **little-endian** byte ordering: the least significant byte (LSB) is stored at the lowest memory address. For the 32-bit value `0x40098000`: byte 0 (lowest address) = `0x00` (LSB), byte 1 = `0x80`, byte 2 = `0x09`, byte 3 = `0x40` (MSB). We write numbers with the MSB first (big-endian notation), but the processor stores them LSB-first. This is a fundamental property of the ARM architecture that affects how you read multi-byte values in hex editors and GDB `x/bx` output.

3. **If you changed the hw pointer at 0x2000062c from 0x40098000 to 0x40090000 using GDB, what I²C controller would the program use? What would happen to the LCD?**
   The program would use **I²C0** instead of I²C1, because all subsequent hardware register accesses (via `i2c1_inst.hw->...`) would read/write the I²C0 registers at `0x40090000`. However, the LCD is physically wired to the I²C1 pins (GPIO 14 for SDA, GPIO 15 for SCL), and those GPIOs are configured for the I²C1 peripheral. The I²C0 controller drives different default pins (GPIO 0/1). So the program would send I²C commands through the wrong controller on the wrong pins — the LCD would receive no signals and would stop updating, displaying whatever was last written before the pointer change.

4. **The macro chain has 4 levels of indirection (I2C_PORT → i2c1 → &i2c1_inst → hw → registers). Is this typical for embedded SDKs? What are the trade-offs of this approach?**
   Yes, this is typical. STM32 HAL, Nordic nRF5 SDK, ESP-IDF, and most professional embedded SDKs use similar multi-level abstractions. **Benefits:** code reuse across multiple peripheral instances, clean type-safe APIs, portability across chip revisions, and testability (you can mock the struct for unit tests). **Costs:** complexity for reverse engineers (harder to trace from API call to hardware), potential code bloat if not optimized, and a steeper learning curve for SDK users. In practice, modern compilers (with `-O2` or higher) optimize away most indirection — the final binary often inlines the pointer dereferences into direct register accesses, so the runtime overhead is negligible.

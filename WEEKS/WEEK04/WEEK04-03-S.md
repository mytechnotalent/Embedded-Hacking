# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 4
Variables in Embedded Systems: Debugging and Hacking Variables w/ GPIO Output Basics

### Non-Credit Practice Exercise 3 Solution: Analyze and Understand GPIO Control

#### Answers

##### Main Function Decompiled (After Renaming)

```c
int main(void)
{
    stdio_init_all();
    gpio_init(0x10);              // Initialize GPIO 16
    gpio_set_dir(0x10, 1);        // Set as output (GPIO_OUT = 1)
    
    while (true) {
        printf("age: %d\r\n", 0); // Uninitialized variable = 0
        gpio_put(0x10, 1);        // LED ON
        sleep_ms(0x1f4);          // Wait 500ms
        gpio_put(0x10, 0);        // LED OFF
        sleep_ms(0x1f4);          // Wait 500ms
    }
}
```

##### GPIO Function Identification Table

| Auto-Generated Name | Actual Function  | Parameters                     | Identification Method                |
|---------------------|-----------------|--------------------------------|--------------------------------------|
| FUN_100030cc        | stdio_init_all  | (void)                         | Called first, no parameters          |
| FUN_xxxxxxxx        | gpio_init       | (uint gpio)                    | Single parameter = pin number (0x10) |
| FUN_xxxxxxxx        | gpio_set_dir    | (uint gpio, bool out)          | Two params: pin + direction (1=out)  |
| FUN_xxxxxxxx        | gpio_put        | (uint gpio, bool value)        | Two params: pin + value (0 or 1)     |
| FUN_xxxxxxxx        | sleep_ms        | (uint32_t ms)                  | Single param = delay (0x1f4 = 500)   |
| FUN_10003100        | printf          | (const char *fmt, ...)         | Format string + varargs              |

##### Key Value Conversions

| Hex    | Decimal | Binary       | Purpose                |
|--------|---------|-------------|------------------------|
| 0x10   | 16      | 0000 1000   | GPIO pin number (red LED) |
| 0x1f4  | 500     | —           | Sleep duration (ms)    |
| 0x01   | 1       | 0000 0001   | GPIO_OUT / LED ON      |
| 0x00   | 0       | 0000 0000   | GPIO_IN / LED OFF      |

```
0x10 = (1 × 16) + 0 = 16
0x1f4 = (1 × 256) + (15 × 16) + 4 = 256 + 240 + 4 = 500
```

##### GPIO Memory Map

| Address      | Register     | Purpose                      |
|-------------|-------------|------------------------------|
| 0x40028000  | IO_BANK0    | GPIO function selection      |
| 0x40038000  | PADS_BANK0  | GPIO pad configuration       |
| 0xd0000000  | SIO         | Single-cycle I/O             |

GPIO 16 specific:
- Pad control: `0x40038000 + (16 × 4) = 0x40038040`
- Function select: `0x40028000 + (16 × 4) = 0x40028040`

##### gpio_put Coprocessor Instruction

```assembly
mcrr    p0, #4, r4, r5, c0
```

- `mcrr` = Move to Coprocessor from two ARM Registers
- `p0` = Coprocessor 0 (GPIO coprocessor on RP2350)
- `r4` = GPIO pin number, `r5` = value (0 or 1)
- Single-cycle GPIO operation

##### Blink Timing Analysis

- ON duration: `sleep_ms(0x1f4)` = 500ms
- OFF duration: `sleep_ms(0x1f4)` = 500ms
- Total cycle: 1000ms = 1 second
- Blink rate: **1 Hz**

#### Reflection Answers

1. **Why does gpio_init() need to configure both PADS_BANK0 and IO_BANK0 registers?**
   These registers control different aspects of GPIO operation. **PADS_BANK0** (`0x40038000`) configures the physical pad properties: input enable (IE), output disable (OD), pull-up/pull-down resistors, drive strength, and slew rate. **IO_BANK0** (`0x40028000`) configures the function multiplexer (FUNCSEL), selecting which internal peripheral drives the pin—SIO (function 5) for software control, UART, SPI, I2C, PWM, etc. Both must be configured: PADS sets the electrical characteristics of the physical pin, while IO_BANK0 routes the correct internal signal to it.

2. **What is the advantage of using the GPIO coprocessor instruction (`mcrr`) instead of writing to memory-mapped registers?**
   The `mcrr` coprocessor instruction completes in a **single CPU cycle**, whereas writing to memory-mapped GPIO registers requires multiple cycles: an address load, a data load, and a store instruction (plus potential bus wait states). On the RP2350, the SIO coprocessor provides deterministic, single-cycle access to GPIO outputs, which is critical for bit-banging protocols (like SPI or custom serial) where precise timing is required. The coprocessor path bypasses the AHB/APB bus entirely.

3. **If you wanted to blink the LED at 10 Hz instead of 1 Hz, what value should `sleep_ms()` use?**
   At 10 Hz, each full on/off cycle is 100ms. Since the loop has two `sleep_ms()` calls (one for ON, one for OFF), each should be `100 / 2 = 50ms`. In hex: `50 = 0x32`. So both `sleep_ms()` calls should use `sleep_ms(0x32)`.

4. **What would happen if you called `gpio_put()` on a pin that hasn't been initialized with `gpio_init()` first?**
   The GPIO pin's function would still be set to its reset default (typically NULL/function 0), not SIO (function 5). The `gpio_put()` coprocessor instruction would update the SIO output register internally, but since the pin's function multiplexer isn't routing SIO to the physical pad, the electrical state of the pin wouldn't change. The LED would remain off. Additionally, without pad configuration, input enable and output disable bits may not be set correctly, further preventing any observable output.

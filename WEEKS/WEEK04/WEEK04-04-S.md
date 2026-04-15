# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 4
Variables in Embedded Systems: Debugging and Hacking Variables w/ GPIO Output Basics

### Non-Credit Practice Exercise 4 Solution: Patch GPIO Binary to Change LED Pin

#### Answers

##### Patch Summary

| Item           | Original         | Patched          | Hex Change    |
|---------------|-----------------|-----------------|---------------|
| LED pin       | GPIO 16          | GPIO 17          | 0x10 → 0x11  |
| Printed value | 0 (uninitialized)| 66               | 0x00 → 0x42  |
| Blink timing  | 500ms            | 100ms            | 0x1f4 → 0x64 |

##### Detailed Patch Locations

**1. gpio_init parameter:**
```assembly
; Before: movs r0, #0x10    (bytes: 10 20)
; After:  movs r0, #0x11    (bytes: 11 20)
```

**2. gpio_set_dir parameter:**
```assembly
; Before: movs r3, #0x10    (bytes: 10 23)
; After:  movs r3, #0x11    (bytes: 11 23)
```

**3. gpio_put (LED ON) parameter:**
```assembly
; Before: movs r4, #0x10    (bytes: 10 24)
; After:  movs r4, #0x11    (bytes: 11 24)
```

**4. gpio_put (LED OFF) parameter:**
```assembly
; Before: movs r4, #0x10    (bytes: 10 24)
; After:  movs r4, #0x11    (bytes: 11 24)
```

**5. printf value:**
```assembly
; Before: movs r1, #0x00    (bytes: 00 21)
; After:  movs r1, #0x42    (bytes: 42 21)
```

**6. sleep_ms (both calls):**
```assembly
; Before: loads 0x1f4 (500ms)
; After:  movs r0, #0x64 (100ms)
```

##### Hex Conversions

```
GPIO 17:  17 = 0x11 = 0001 0001 binary
Value 66: 66 = 0x42 = 0100 0010 binary
100ms:   100 = 0x64 = 0110 0100 binary
```

##### Decompiled Result After All Patches

```c
int main(void)
{
    stdio_init_all();
    gpio_init(0x11);              // GPIO 17 (green LED)
    gpio_set_dir(0x11, 1);        // Output
    
    while (true) {
        printf("age: %d\r\n", 0x42);  // Prints 66
        gpio_put(0x11, 1);            // Green LED ON
        sleep_ms(0x64);               // 100ms
        gpio_put(0x11, 0);            // Green LED OFF
        sleep_ms(0x64);               // 100ms
    }
}
```

##### Hardware Verification

- GREEN LED (GPIO 17) blinks at 10 Hz (100ms on, 100ms off)
- RED LED (GPIO 16) remains off
- Serial output: `age: 66` repeating

#### Reflection Answers

1. **Why did we need to patch GPIO 16 in multiple places (gpio_init, gpio_set_dir, gpio_put)?**
   Each function takes the GPIO pin number as a separate parameter. `gpio_init(16)` configures the pad and function mux for pin 16. `gpio_set_dir(16, 1)` sets pin 16's direction to output. `gpio_put(16, value)` toggles pin 16's output state. These are independent function calls with independent immediate values in the instruction stream—the compiler doesn't share or reuse a single "pin number variable." Each `movs rN, #0x10` loads the pin number fresh for its respective function call. Missing any one patch would result in a mismatch: e.g., initializing pin 17 but toggling pin 16.

2. **What would happen if you forgot to patch one of the gpio_put calls?**
   You would get asymmetric behavior. For example, if you patched the "LED ON" `gpio_put` to pin 17 but left the "LED OFF" at pin 16: GPIO 17 (green) would turn on but never turn off (staying permanently lit), while GPIO 16 (red) would receive the "off" command for a pin that was never initialized—which would have no visible effect. The result: green LED stuck on, no blinking.

3. **How does the instruction encoding differ for immediate values less than 256 vs. greater than 255?**
   In 16-bit Thumb encoding, `movs Rd, #imm8` can only encode immediate values 0–255 in a single 2-byte instruction. For values > 255 (like 500 = 0x1f4), the compiler must use either: (a) a 32-bit Thumb-2 `movw Rd, #imm16` instruction (4 bytes, can encode 0–65535), (b) a multi-instruction sequence that constructs the value (e.g., `movs` + `lsls` + `add`), or (c) an `ldr Rd, [pc, #offset]` that loads the constant from a literal pool in flash. This is why patching `sleep_ms(500)` may be more complex than patching `gpio_put(16, 1)`.

4. **If you wanted to make the LED blink at exactly 5 Hz, what sleep_ms value would you use?**
   At 5 Hz, each complete cycle is `1000 / 5 = 200ms`. With two `sleep_ms()` calls per cycle (ON and OFF), each call should be `200 / 2 = 100ms`. In hex: `100 = 0x64`. So `sleep_ms(0x64)` for each call—which is exactly the value used in this exercise's patch. For a different duty cycle (e.g., 150ms on, 50ms off), you'd use different values for each call while keeping the total at 200ms.

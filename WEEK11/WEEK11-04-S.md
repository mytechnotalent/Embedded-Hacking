# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 11
Functions in Embedded Systems: Debugging and Hacking Functions w/ IR Remote and Multi-LED Control

### Non-Credit Practice Exercise 4 Solution: Change Blink Speed

#### Answers

##### Delay Parameter

| Parameter | Original     | Patched       |
|----------|-------------|--------------|
| Delay     | 50ms         | 25ms          |
| Hex       | 0x32         | 0x19          |
| Register  | r2           | r2            |
| Instruction | movs r2, #0x32 | movs r2, #0x19 |

##### Assembly Context

```asm
movs r0, <pin>       ; r0 = GPIO pin
movs r1, #3          ; r1 = blink count
movs r2, #0x32       ; r2 = delay 50ms ← PATCH THIS
bl   blink_led       ; blink_led(pin, 3, 50)
```

##### Patch

```
Before: 32 22  (movs r2, #0x32 = 50ms)
After:  19 22  (movs r2, #0x19 = 25ms)
```

**Warning:** The byte `0x32` is also ASCII '2'. Verify you're patching the correct `movs r2` instruction by checking surrounding bytes — `movs r1, #3` (`03 21`) should appear immediately before, and `bl blink_led` immediately after.

##### Timing Comparison

| Metric            | Original (50ms) | Patched (25ms)  |
|-------------------|-----------------|----------------|
| On-time per blink | 50ms            | 25ms           |
| Off-time per blink| 50ms            | 25ms           |
| One blink cycle   | 100ms           | 50ms           |
| 3 blinks total    | 300ms           | 150ms          |
| Perceived speed   | Normal          | 2× faster      |

#### Reflection Answers

1. **If delay = 1ms (0x01), would you still see the LED blink, or would it appear constantly on?**
   At 1ms on/off (2ms per cycle, 500Hz flicker), the LED would appear **constantly on** to the human eye. Human flicker fusion threshold is approximately 60Hz — anything above that appears as a steady light. At 500Hz, the LED is switching far too fast for the eye to perceive individual blinks. The LED would look like it's at roughly 50% brightness (since it's on half the time) compared to being fully on. The 3 blinks would complete in just 6ms total, appearing as a brief flash rather than distinct blinks.

2. **0x32 appears as both the delay value (50ms) and potentially ASCII '2'. How would you systematically find ALL occurrences of 0x32 and determine which to patch?**
   Search the binary for all `0x32` bytes, then examine the **context** of each occurrence: (1) Check the byte following `0x32` — if it's `0x22`, this is `movs r2, #0x32` (the delay parameter). If it's `0x2C`, it's `cmp r4, #0x32` (comparing against ASCII '2'). (2) Examine surrounding instructions: the delay `0x32` will be preceded by `movs r1, #3` (blink count) and followed by `bl blink_led`. A comparison `0x32` will be near `beq`/`bne` branches. (3) Use GDB to disassemble the region (`x/10i <addr-4>`) and read the instruction mnemonic. (4) Cross-reference with the function structure — delay patches are in `blink_led` call setup, comparisons are in `ir_to_led_number` or similar dispatcher functions.

3. **For a delay of 500ms (0x1F4), the value won't fit in a movs immediate (max 255). How would the compiler handle it?**
   For 500 (`0x1F4`), the compiler would use either: (1) A 32-bit `movw r2, #0x1F4` Thumb-2 instruction (4 bytes), which can encode any 16-bit immediate (0–65535). (2) A literal pool load: `ldr r2, [pc, #offset]` that reads `0x000001F4` from a nearby data word. The `movw` approach is preferred for values 256–65535 because it's a single instruction with no data dependency. For values exceeding 65535, a literal pool or `movw`+`movt` pair would be necessary.

4. **The blink function uses the delay for both on-time and off-time (symmetrical blink). Could you make the LED stay on longer than off by patching only one instruction?**
   Not with a single patch to the `movs r2` instruction, because `blink_led` uses the same delay parameter for both the on-phase and off-phase `sleep_ms` calls internally. To create asymmetric blink timing, you would need to patch **inside** the `blink_led` function itself — find the two `sleep_ms` calls within the blink loop and modify their delay arguments independently. For example, find the `ldr`/`movs` that sets up `r0` before each `bl sleep_ms` inside `blink_led`, and patch one to a different value. This would require disassembling `blink_led` to locate both `sleep_ms` call sites.

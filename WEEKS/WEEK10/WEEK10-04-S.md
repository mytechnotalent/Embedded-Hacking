# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 10
Dynamic Conditionals in Embedded Systems: Debugging and Hacking Dynamic Conditionals w/ SG90 Servo Basics

### Non-Credit Practice Exercise 4 Solution: Speed Profile

#### Answers

##### Sleep Duration Values

| Parameter    | Original | Case 1 (Fast Snap) | Case 2 (Slow Sweep) |
|-------------|---------|--------------------|--------------------|
| Duration     | 500 ms   | 100 ms              | 1000 ms            |
| Hex          | 0x1F4    | 0x64                | 0x3E8              |
| LE Bytes     | F4 01 00 00 | 64 00 00 00     | E8 03 00 00        |

##### Patch Case 1: 500ms → 100ms (Fast Snap)

```
Before: F4 01 00 00  (500ms)
After:  64 00 00 00  (100ms)
```

##### Patch Case 2: 500ms → 1000ms (Slow Sweep)

```
Before: F4 01 00 00  (500ms)
After:  E8 03 00 00  (1000ms)
```

##### Literal Pool Considerations

If the compiler shares a single literal pool entry for `0x000001F4` across both cases, you **cannot** patch them independently without additional work. Verify by checking whether case 1 and case 2 `ldr` instructions reference the same pool address. If shared, you need to find unused space in the binary for a second value or repurpose another unused literal pool entry.

##### Behavior After Patch

| Key | Original           | Patched                           |
|-----|-------------------|----------------------------------|
| '1' | 500ms between moves | 100ms — near-instantaneous snap   |
| '2' | 500ms between moves | 1000ms — slow, deliberate sweep   |

#### Reflection Answers

1. **Why does 100ms feel like an instant "snap" while 1000ms feels like a smooth sweep? The servo moves the same distance either way.**
   Human perception of motion depends on the **pause between position updates**, not the motor speed. At 100ms delay, the servo reaches each angle before the next one is set — the positions update so quickly that the motion appears continuous and instant. At 1000ms delay, there's a full second between movements, so you can see the servo pause at each intermediate angle. The SG90 servo physically takes about 200–300ms to traverse its full range at no load, so 100ms is faster than the travel time (the servo is still moving when the next command arrives), creating a snappy feel. At 1000ms, the servo has already completed its move and waits idle before the next command.

2. **If both cases share the same literal pool entry for 500ms, what strategy would you use to give them different sleep values?**
   Several approaches: (1) **Find unused literal pool space** — look for entries that are no longer referenced and overwrite one with `0x64` (100ms) while keeping the other for `0x3E8` (1000ms). (2) **Repurpose an existing value** — if another constant in the pool happens to equal your desired value, redirect the `ldr` offset to point there. (3) **Change the `ldr` to a `movs`** — for values ≤ 255 (like 100), replace the 4-byte `ldr r0, [pc, #offset]` with `movs r0, #0x64` (2 bytes) + `nop` (2 bytes) for padding. This works for case 1 (100 fits in 8 bits) but not case 2 (1000 exceeds 255).

3. **What is the minimum sleep_ms value where the SG90 servo can actually complete a full 0°–180° sweep before the next command?**
   The SG90 servo has a rated speed of approximately 0.12 seconds per 60° at 4.8V. For a full 180° sweep: 0.12 × (180/60) = 0.12 × 3 = **0.36 seconds (360ms)**. In practice, with load and signal processing overhead, **400–500ms** is a safe minimum for reliable full-range travel. Below this, the servo may not reach the target angle before the next position command arrives, resulting in incomplete movements or jittery behavior. The original 500ms value was chosen to reliably allow full travel.

4. **What would happen if you set sleep_ms to 0 for both cases? How would the servo physically behave?**
   With `sleep_ms(0)`, the loop runs at full CPU speed, sending angle commands as fast as the processor can execute. The servo would receive thousands of position updates per second, alternating between two angles. Physically, the servo would **vibrate or oscillate** — it never has time to reach either target angle before being told to go to the other one. The PWM signal would switch so rapidly that the servo's control circuit would see constantly changing targets, producing a buzzing sound and erratic oscillation near the midpoint. This could also overheat the servo motor due to constant direction changes.

# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 10
Dynamic Conditionals in Embedded Systems: Debugging and Hacking Dynamic Conditionals w/ SG90 Servo Basics

### Non-Credit Practice Exercise 3 Solution: Reverse the Servo Direction

#### Answers

##### Four Literal Pool Swaps

| Patch | Location       | Original          | Patched           |
|-------|---------------|-------------------|-------------------|
| Case 1 Angle 1 | Literal pool | 00 00 00 00 (0.0f)   | 00 00 34 43 (180.0f) |
| Case 1 Angle 2 | Literal pool | 00 00 34 43 (180.0f) | 00 00 00 00 (0.0f)   |
| Case 2 Angle 1 | Literal pool | 00 00 34 43 (180.0f) | 00 00 00 00 (0.0f)   |
| Case 2 Angle 2 | Literal pool | 00 00 00 00 (0.0f)   | 00 00 34 43 (180.0f) |

##### Behavior After Patch

| Key | Original             | Patched              |
|-----|---------------------|---------------------|
| '1' | 0° → 180° (sweep up) | 180° → 0° (sweep down) |
| '2' | 180° → 0° (sweep down) | 0° → 180° (sweep up) |

The terminal output text ("Moving to 180..." / "Moving to 0...") remains unchanged — it still says the original directions. Only the physical servo behavior is reversed.

##### GDB Verification

```gdb
(gdb) x/4wx <literal_pool_start>
```

Examine all angle entries in the literal pool to identify which 4-byte words to swap.

#### Reflection Answers

1. **After this patch, the serial output still says "Moving to 180" when the servo actually moves to 0. Why is this a security concern? What real-world attack does this mimic?**
   This is a classic **display spoofing** attack. The user interface (serial output) shows one thing while the hardware does another. In real-world systems, this mimics attacks on SCADA/ICS systems where operator displays show "normal" readings while the physical process is manipulated (similar to Stuxnet, which showed normal centrifuge speeds while actually damaging them). In medical devices, this could display a safe dosage while delivering a different amount. The lesson is that **you cannot trust the display if the firmware has been tampered with** — the display text and the actual behavior are patched independently.

2. **Instead of swapping the data values, could you achieve the same result by swapping the branch targets (making case '1' jump to case '2' code and vice versa)? What are the trade-offs?**
   Yes, you could swap the `beq` target addresses so that when the user presses '1', execution jumps to the case '2' code path and vice versa. **Trade-offs:** Swapping branch targets changes the instructions (modifying the offset bytes in `beq`), which is more complex — you need to correctly calculate the new PC-relative offsets. Swapping data values is simpler (just exchange 4-byte float values) and less error-prone. However, swapping branches would also swap the printf messages, so "Moving to 180" would display for the path that actually moves to 180 — keeping the display consistent. The data-swap approach intentionally creates a mismatch between display and behavior.

3. **If the compiler shares a single literal pool entry for 0x43340000 (180.0f) across both cases, how does swapping that one entry affect the behavior?**
   If the compiler optimized by sharing a single `0x43340000` literal pool entry for all references to 180.0f, then both case '1' and case '2' load from the same address. Changing that one entry to `0x00000000` (0.0f) would affect **both** cases simultaneously — they would both use 0.0f where they originally used 180.0f. Similarly, if there's only one `0x00000000` entry shared, changing it affects both cases. You would need to verify whether each case uses its own pool entry or shares entries by examining the `ldr` offsets. If shared, you may need to find unused space to create a second copy of the value.

4. **How would you verify the patch is correct without physical hardware? What GDB commands would you use?**
   Set breakpoints before each `bl servo_set_angle` call, then examine `r0` (or `s0`) which holds the angle argument. Run through both cases and verify: (1) `b *<case1_servo_call>` → `c` → press '1' → `info float` or `p $s0` — should show 180.0f (was 0.0f). (2) Continue to second call — should show 0.0f (was 180.0f). Repeat for case '2'. You can also examine the literal pool directly: `x/wx <pool_addr>` to verify the bytes were swapped. Additionally, `x/f <pool_addr>` displays the value as a float for quick verification.

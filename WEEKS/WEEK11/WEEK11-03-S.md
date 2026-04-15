# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 11
Functions in Embedded Systems: Debugging and Hacking Functions w/ IR Remote and Multi-LED Control

### Non-Credit Practice Exercise 3 Solution: Swap All Three LEDs

#### Answers

##### GPIO Rotation Patch

| Struct Member | Original         | Patched          | Effect              |
|--------------|-----------------|-----------------|---------------------|
| led1_pin     | 0x10 (GPIO 16 Red)   | 0x11 (GPIO 17 Green) | Button 1 → Green |
| led2_pin     | 0x11 (GPIO 17 Green) | 0x12 (GPIO 18 Yellow) | Button 2 → Yellow |
| led3_pin     | 0x12 (GPIO 18 Yellow) | 0x10 (GPIO 16 Red)   | Button 3 → Red   |

##### Assembly Patches

Three single-byte patches in `movs` immediate fields:

```
Patch 1 (led1_pin): 10 → 11
Before: 10 20  (movs r0, #0x10)
After:  11 20  (movs r0, #0x11)

Patch 2 (led2_pin): 11 → 12
Before: 11 20  (movs r0, #0x11)
After:  12 20  (movs r0, #0x12)

Patch 3 (led3_pin): 12 → 10
Before: 12 20  (movs r0, #0x12)
After:  10 20  (movs r0, #0x10)
```

##### GDB Verification

```gdb
(gdb) b *0x10000280
(gdb) c
(gdb) x/6bx <struct_address>
```

Before patch: `10 11 12 00 00 00`
After patch: `11 12 10 00 00 00`

##### Behavior After Patch

| Button (IR) | NEC Code | Terminal Log              | Actual LED |
|------------|----------|---------------------------|-----------|
| Button 1   | 0x0C     | "LED 1 activated on GPIO 16" | Green (GPIO 17) |
| Button 2   | 0x18     | "LED 2 activated on GPIO 17" | Yellow (GPIO 18) |
| Button 3   | 0x5E     | "LED 3 activated on GPIO 18" | Red (GPIO 16) |

The terminal logs are **desynchronized** from actual behavior.

#### Reflection Answers

1. **Terminal log still says "LED 1 activated on GPIO 16" even though GPIO 17 (Green) is actually blinking. Why don't the logs update automatically?**
   The `printf` format strings and their arguments are separate from the struct pin assignments. The log message "LED 1 activated on GPIO 16" is generated from hardcoded format strings or from reading the **original** pin value before our patch takes effect. The GPIO number in the log comes from a different code path — likely a format string like `"LED %d activated on GPIO %d\r\n"` where the GPIO value was loaded from the struct at a different point or is computed independently. Since we only patched the `movs` instructions that store pin values into the struct, the logging code still uses whatever values it computes independently.

2. **If the struct initialization used ldr from a literal pool instead of movs immediates, how would the patching differ?**
   With literal pool loads, the pin values would be stored as 32-bit words in a data area near the function code. You would need to: (1) find the `ldr r0, [pc, #offset]` instruction, (2) calculate the PC-relative offset to locate the literal pool entry, (3) navigate to the pool address in the hex editor, and (4) modify the 4-byte value there. For example, GPIO 16 would be `10 00 00 00` (little-endian) in the pool. This is more work than patching a 1-byte `movs` immediate, and you'd need to verify no other code shares the same pool entry. The `movs` approach is simpler because the value is encoded directly in the instruction.

3. **Could you achieve the same LED rotation by patching gpio_init/gpio_put calls instead of the struct initialization? Which approach is cleaner?**
   Patching `gpio_init` and `gpio_put` calls would require finding every call site that references each GPIO pin and modifying the pin argument. This is scattered throughout multiple functions (`process_ir_led_command`, `blink_led`, `leds_all_off`). The struct initialization approach is **far cleaner** — three adjacent `movs` instructions in one location control the entire mapping. By patching the struct data at its source, every function that reads from the struct automatically gets the new values. This demonstrates the power of data-driven design: changing the data at one point affects all code that uses it.

4. **In a real attack, why is log desynchronization (display says one thing, hardware does another) dangerous for forensic analysis?**
   Log desynchronization is dangerous because forensic investigators rely on logs to reconstruct what happened. If logs show "LED 1 on GPIO 16" but the hardware actually activated GPIO 17, investigators would draw incorrect conclusions about which physical device was controlled. In industrial systems, this could mask sabotage — operators see "normal" readings while equipment is being misused. In security systems, tampered firmware could log "door locked" while actually unlocking it. The logs become actively misleading, not just incomplete. This is a form of **anti-forensics** that makes post-incident analysis unreliable and can delay or prevent discovery of the actual attack.

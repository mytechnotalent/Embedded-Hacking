# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 11
Functions in Embedded Systems: Debugging and Hacking Functions w/ IR Remote and Multi-LED Control

### Non-Credit Practice Exercise 1 Solution: Add a Fourth LED

#### Answers

##### Struct Layout (simple_led_ctrl_t)

| Offset | Field       | Size   | Original Value | Hex  |
|--------|------------|--------|----------------|------|
| 0      | led1_pin   | 1 byte | GPIO 16        | 0x10 |
| 1      | led2_pin   | 1 byte | GPIO 17        | 0x11 |
| 2      | led3_pin   | 1 byte | GPIO 18        | 0x12 |
| 3      | led1_state | 1 byte | false (0)      | 0x00 |
| 4      | led2_state | 1 byte | false (0)      | 0x00 |
| 5      | led3_state | 1 byte | false (0)      | 0x00 |

Total struct size: **6 bytes** (3 pin bytes + 3 state bytes).

##### Assembly Initialization Pattern

```asm
movs r0, #0x10      ; led1_pin = 16
strb r0, [r4, #0]   ; struct offset 0
movs r0, #0x11      ; led2_pin = 17
strb r0, [r4, #1]   ; struct offset 1
movs r0, #0x12      ; led3_pin = 18
strb r0, [r4, #2]   ; struct offset 2
```

##### Patch: Add GPIO 19 at Struct Offset 3

Writing `0x13` to offset 3 **overwrites led1_state**:

| Offset | Before       | After        | Impact                |
|--------|-------------|-------------|----------------------|
| 3      | 0x00 (led1_state = false) | 0x13 (led4_pin = GPIO 19) | led1_state corrupted |

##### GDB Verification

```gdb
(gdb) b *0x10000280
(gdb) c
(gdb) x/8bx <struct_address>
```

Before patch: `10 11 12 00 00 00`
After patch: `10 11 12 13 00 00`

#### Reflection Answers

1. **The original struct has 6 members (3 pins + 3 states) in 6 bytes. If you add a fourth pin at offset 3, you overwrite led1_state. What is the practical impact on LED 1 behavior?**
   The byte `0x13` (decimal 19) is written to offset 3, which the program reads as `led1_state`. Since `bool` in C treats any non-zero value as `true`, `led1_state` would be interpreted as `true` (on) immediately after the struct is initialized. LED 1 would appear to be in the "on" state from the start, regardless of whether the user pressed button 1. The `leds_all_off` function may reset it to 0, but every time the struct is re-initialized on the stack (each loop iteration), the corrupted state returns. The fourth LED at GPIO 19 would need additional `gpio_init` and `gpio_set_dir` calls to actually function — just writing the pin number into the struct doesn't configure the GPIO hardware.

2. **How would you verify the exact struct layout and offsets using GDB's memory examination commands?**
   Set a breakpoint after struct initialization (`b *0x10000280`), then `x/6bx <struct_base>` to see all 6 bytes. Verify: offsets 0–2 should show `10 11 12` (pin values), offsets 3–5 should show `00 00 00` (state values). Use `x/1bx <struct_base+N>` for individual fields. To find the struct base, examine `r4` at the breakpoint since the `strb r0, [r4, #N]` instructions use r4 as the base. You can also use `p/x $r4` to get the base address, then `x/6bx $r4` for the complete layout.

3. **If the get_led_pin function uses a bounds check (e.g., if led_num > 3 return 0), what additional patch would you need?**
   You would need to find the comparison instruction in `get_led_pin` (at approximately `0x100002a0`) — likely a `cmp rN, #3` followed by a conditional branch. Patch the immediate from `#3` to `#4` so the bounds check allows led_num = 4. For example, if the check is `cmp r1, #3; bhi default`, change `03` to `04` in the `cmp` instruction's immediate byte. Without this patch, passing led_num=4 would fail the bounds check and return 0 (no pin), so the fourth LED would never be addressed.

4. **Could you extend the struct without overwriting existing fields by finding free space elsewhere in the binary? What challenges would that introduce?**
   You could find unused space (padding, NOP sleds, or unused data) and place the extended struct there. However, this introduces major challenges: (1) Every instruction that references the original struct address via `r4` would need to be redirected to the new location. (2) All `strb`/`ldrb` offsets would need updating. (3) Stack-allocated structs are recreated each loop iteration — you'd need to change the stack frame size (`sub sp, sp, #N`). (4) Functions that receive the struct pointer as an argument would need their call sites updated. In practice, relocating a struct in a compiled binary is extremely complex and error-prone — overwriting adjacent fields is the pragmatic (if destructive) approach.

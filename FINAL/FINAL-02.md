# Final Project Option 2: Operation Dark Eclipse

## Project Requirements & Grading Criteria

---

## 📋 Project Overview

You are an embedded reverse engineer working for an allied intelligence agency. A hostile underground facility is operating centrifuge cascades controlled by RP2350-based firmware. Your mission is to reverse engineer the centrifuge control binary (`FINAL-02.uf2`), modify it to sabotage the centrifuges (double the motor speed), mask the sabotage from operators (keep green LED on, LCD showing normal readings), and produce a weaponized firmware image — all without the operators detecting any anomaly.

This project covers concepts from **Weeks 1–11**: registers, stack, ARM assembly, memory layout, live debugging, boot process, variables, memory sections, Ghidra, binary patching, integer/float data types, IEEE 754, GPIO inputs, compiler optimizations, constants, I²C, LCD displays, operators, floating-point hacking, conditionals, PWM, servo control, branch modification, NOP-ing, structures, functions, and log desynchronization.

---

## 🎯 Learning Objectives

Upon completion of this project, you will demonstrate the ability to:

1. Configure Ghidra for ARM Cortex-M33 binary analysis
2. Identify inlined functions in optimized Thumb-2 code (the compiler inlines `update_display()`, `update_leds()`, and `set_motor_speed()` directly into `main()`)
3. Recognize RP2350 GPIO coprocessor instructions (`mcrr`) as `gpio_put` equivalents
4. Locate and modify immediate values controlling sweep range, LED thresholds, and display constants
5. Separate "actual" speed from "displayed" speed in a compiled binary to achieve log desynchronization
6. Export a patched binary and convert it to UF2 format
7. Analyze the ethical dimensions of offensive cyber operations
8. Compare your attack methodology to STUXNET and articulate defensive countermeasures

---

## 📦 Deliverables Checklist

You must submit **all** of the following. Missing deliverables will result in zero points for the corresponding task.

| # | Deliverable | Format | Task |
|---|------------|--------|------|
| 1 | Ghidra project screenshot showing project name (`Dark_Eclipse`), processor settings (ARM Cortex, little endian), and base address (`0x10000000`) | PNG/JPG | Task 1 |
| 2 | Address table listing `main()`, `servo_set_angle()`, LED GPIO operations, LCD operations, sweep loop start, and sweep direction logic | Markdown table or text | Task 1 |
| 3 | Speed doubling analysis: addresses of sweep limit comparison and endpoint assignment, original bytes, patched bytes, and method explanation | Markdown/text | Task 2 |
| 4 | Overspeed masking analysis: addresses of LED control logic, original bytes, patched bytes (or explanation of why no patch is needed if using surgical method) | Markdown/text | Task 3 |
| 5 | Display falsification analysis: address of display constant, original bytes, patched bytes (or explanation of why no patch is needed if using surgical method) | Markdown/text | Task 4 |
| 6 | `FINAL-02_compromised.bin` — the exported patched binary | BIN file | Task 5 |
| 7 | `FINAL-02_compromised.uf2` — the UF2-converted binary | UF2 file | Task 5 |
| 8 | Verification report: description of observed servo sweep range, LED states, LCD reading, and serial output after flashing | Markdown/text | Task 5 |
| 9 | Written analysis: STUXNET comparison (500 words), ethical analysis (300 words), defensive measures (3 proposals) | Markdown/text | Questions 1–3 |

---

## 📊 Grading Rubric — Detailed Breakdown

### Total Points: 130

---

### Task 1: Initial Analysis — 15 points

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Ghidra project created with correct name (`Dark_Eclipse`) | 3 | Project name matches exactly | Minor naming deviation | No project created |
| Processor configured as ARM Cortex 32-bit, little endian | 3 | Correct processor and endianness selected | Correct processor, wrong endianness | Wrong processor entirely |
| Base address set to `0x10000000` | 3 | Correct base address | Off by a nibble | Wrong or default base address |
| `main()` function identified with correct address | 3 | Correct address documented | Within one function of correct | Not found |
| Sweep loop identified: loop body start address AND direction toggle logic located | 2 | Both sweep loop start and direction change documented | Only one of the two documented | Neither found |
| At least 3 additional key addresses documented (e.g., `servo_set_angle`, `lcd_puts`, `sleep_ms`, `snprintf`, LED GPIO operations) | 1 | 3+ addresses correct | 1–2 addresses correct | None found |

---

### Task 2: Double the Motor Speed — 25 points

**Objective:** The servo must sweep 0°→160°→0° instead of 0°→80°→0°.

**Multiple valid methods exist.** Full credit is awarded for ANY method that achieves the objective with correct documentation.

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Identified the correct location(s) for speed modification | 10 | Correct address(es) documented; instruction(s) identified with hex bytes; explains which instructions control sweep range | Found the right area but wrong specific instruction | Not found |
| Correct patch applied and verified | 10 | Patch bytes documented (before/after); binary runs and servo sweeps to 160° | Patch concept is correct but byte value or address is slightly off | No patch or patch breaks binary |
| Documentation of method and reasoning | 5 | Explains WHY the chosen instructions control the sweep range; traces data flow from constant → comparison → endpoint assignment | States what was changed but not why | No documentation |

**Accepted methods (full credit for any):**

| Method | Description | Minimum Patches |
|--------|-------------|-----------------|
| A — Surgical (recommended) | Change only sweep limit `cmp` and sweep endpoint `movs` | 2 instructions, 3 bytes |
| B — Brute force | Change all occurrences of `0x50` (TARGET_SPEED) to `0xA0` | 5+ instructions (requires fixing display/LEDs separately) |
| C — Multiply | Insert a `vadd.f32 s15,s15,s15` before servo call | Requires careful instruction replacement without expanding binary |

---

### Task 3: Mask the Overspeed Warning — 25 points

**Objective:** Green LED must remain ON at all times. Red LED must NEVER illuminate, regardless of actual speed.

**IMPORTANT:** If you used **Method A (surgical)** from Task 2, the green LED stays on automatically because `displayed_speed` was never changed from 80, which is below the 90 threshold. In this case, you must **explain why** no LED patch is needed for full credit.

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Identified LED control logic correctly (GPIO coprocessor instructions, pin numbers 0x10/0x11, overspeed threshold comparison) | 10 | Documents the `cmp` against threshold (0x5A), the branch instruction, and the GPIO `mcrr` instructions for both LED paths; OR explains why surgical method makes patching unnecessary | Found LED operations but missed the comparison or branch | Not found |
| Correct patch ensures green is always on (or explains why no patch is needed) | 10 | Patch bytes documented (before/after) with method explained; OR clear explanation of why Method A makes the display path always take the green branch (displayed_speed 80 < threshold 90) | Green is usually on but red sometimes activates | No patch and no explanation |
| Documentation and verification | 5 | States the expected LED behavior after patching and confirms it matches; OR provides complete logical trace for why surgical method guarantees green | Partial explanation | No documentation |

**Accepted methods (full credit for any):**

| Method | Description |
|--------|-------------|
| Built-in (Method A from Task 2) | No patch needed — explain why `displayed_speed = 80 < threshold 90` keeps green on |
| Swap GPIO pins | Change pin numbers in overspeed path: `0x10`↔`0x11` |
| Raise threshold | Change `cmp r3, #0x5a` to `cmp r3, #0xff` (unreachable) |
| Invert branch | Change `bhi` to `bls` |
| NOP the branch | Replace `bhi` with `nop` (0x00BF) so overspeed path is never taken |

---

### Task 4: Falsify the Display Reading — 25 points

**Objective:** The LCD must display the ORIGINAL speed range (0–80 RPM) while the servo physically sweeps 0°–160°.

**IMPORTANT:** If you used **Method A (surgical)** from Task 2, the LCD already shows "Speed:  80 RPM" automatically because the display constant (`mov.w r12, #0x50`) was never changed. In this case, you must **explain why** no display patch is needed for full credit.

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Identified display/motor separation in the binary: the `displayed_speed` assignment uses a separate inlined constant from the sweep limit | 10 | Documents the `mov.w r12, #0x50` instruction (display constant) as distinct from the `movs r5, #0x50` (sweep endpoint); explains that the compiler inlined TARGET_SPEED as independent immediates; OR traces the `snprintf` argument back to `r12` | Found the display path but did not identify the separation from motor path | Not found |
| Correct patch preserves original display value (or explains why no patch is needed) | 10 | Patch bytes documented; LCD confirmed to show 80 while servo sweeps to 160; OR provides complete logical trace for why Method A inherently separates display from motor | Display sometimes shows correct value, sometimes not | No patch and no explanation |
| Verified LCD shows 80 RPM while servo sweeps to 160° | 5 | Confirms observation: LCD reads "Speed:  80 RPM" during full sweep AND explains why this constitutes successful log desynchronization | Partial verification | No verification |

**Accepted methods (full credit for any):**

| Method | Description |
|--------|-------------|
| Built-in (Method A from Task 2) | No patch needed — explain that the display constant at `0x1000033e` was untouched |
| Restore display constant | If brute-force changed `0x1000033e`, change it back from `0xA0` to `0x50` |
| Right-shift snprintf argument | Replace `mov r3, r12` with `lsr.w r3, r12, #1` before `snprintf` call |

---

### Task 5: Export and Verify — 10 points

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Exported `FINAL-02_compromised.bin` from Ghidra | 2 | File submitted and is a valid binary | File submitted but corrupted | Not submitted |
| Converted to `FINAL-02_compromised.uf2` using `uf2conv.py` with correct flags (`--base 0x10000000 --family 0xe48bff59`) | 2 | UF2 file submitted; correct conversion command documented | UF2 created but wrong base or family | Not submitted |
| Servo sweeps 0°–160° (visibly wider arc than original) | 2 | Confirmed and documented | Sweep is wider but not exactly double | No change observed |
| Green LED always on, red LED never on | 2 | Both confirmed | Green on but red occasionally flickers | LEDs not correct |
| LCD shows "Speed:  80 RPM" constantly while servo sweeps to 160° | 1 | Confirmed | LCD shows correct value intermittently | LCD shows doubled value |
| Serial output shows actual sweep_pos reaching 160 at endpoint | 1 | Confirmed via serial monitor | Serial not checked | Serial shows original values |

---

### Question 1: The STUXNET Parallel — 10 points

**Requirement:** 500 words or less comparing your attack to the original STUXNET worm.

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Accurate comparison of objectives (both target centrifuges, both cause physical destruction via cyber means) | 4 | Specific parallels drawn: target type, method of sabotage, physical outcome through digital means | Generic comparison without specifics | Not addressed |
| Explains why log/display desynchronization is dangerous (operators trust instruments, no visual warning until catastrophic failure) | 3 | Articulates the trust model: humans cannot directly observe centrifuge speed → rely on instruments → when instruments lie, no warning exists | Mentions desynchronization but doesn't explain the trust model | Not addressed |
| Explains why this attack type is nearly impossible to detect remotely (appears as mechanical failure, no malware signatures, attribution-free) | 3 | Discusses: legitimate firmware appearance, no network indicators, failure looks like manufacturing defect or material fatigue | Mentions difficulty of detection but lacks analysis | Not addressed |

---

### Question 2: Ethical Considerations — 10 points

**Requirement:** 300 words or less discussing when offensive cyber operations are justified.

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Analyzes when this type of operation is justified (proportionality, prevention of greater harm, last resort) | 4 | Balanced analysis considering multiple perspectives; discusses proportionality principle | One-sided analysis (either fully for or fully against) | Not addressed |
| Proposes safeguards for offensive cyber operations (legal authorization, proportionality assessment, accountability, minimization of collateral effects) | 3 | Names at least 3 specific safeguards with brief justification | Names 1–2 safeguards | Not addressed |
| Reflects on Agent NIGHTINGALE's sacrifice in the ethical calculus (emotional weight should inform but not override ethical analysis) | 3 | Nuanced reflection: honor through excellence, not by abandoning ethical constraints | Mentions NIGHTINGALE but doesn't integrate into ethical framework | Not addressed |

---

### Question 3: Defensive Measures — 10 points

**Requirement:** Propose THREE technical countermeasures (one hardware-level, one firmware-level, one operational security).

| Criterion | Points | Full Credit | Partial Credit | No Credit |
|-----------|--------|-------------|----------------|-----------|
| Hardware-level protection is practical and specific (e.g., HSM for firmware verification, secure boot, tamper-evident seals, redundant independent sensors) | 3 | Specific technology named, explained how it prevents the attack vector | Generic mention of "better hardware security" | Not addressed |
| Firmware-level protection is practical and specific (e.g., code signing, runtime integrity checks, watchdog comparing sensor readings with motor commands, MPU) | 4 | Specific technology named, explained how it prevents the attack vector | Generic mention of "better firmware security" | Not addressed |
| Operational security improvement is practical and specific (e.g., air-gapped programming, multi-person integrity for updates, independent tachometer, golden image comparison) | 3 | Specific process named, explained how it prevents the attack vector | Generic mention of "better operational security" | Not addressed |

---

## 📋 Partial Credit Policy

The following partial credit guidelines apply across all tasks:

| Scenario | Credit |
|----------|--------|
| Correct concept and approach, wrong specific address or byte value | 75% of task points |
| Identified location correctly but patch is incorrect or breaks binary | 60% of task points |
| Explained concept correctly but could not locate in binary | 40% of task points |
| Patched correctly but could not explain why the fix works | 50% of task points |
| Documented analysis process thoroughly even though result is wrong | 30% of task points |
| Display blank but motor speed doubling works | 20% of display task points |
| Partial speed increase (e.g., 1.5x instead of 2x) | 60% of speed task points |
| Green LED on but red LED occasionally activates | 40% of LED task points |

---

## ⚠️ Common Pitfalls (Read Carefully)

These are the most frequent mistakes. Avoid them.

| Pitfall | Consequence | How to Avoid |
|---------|-------------|--------------|
| Looking for `update_leds` / `update_display` / `set_motor_speed` as separate `bl` calls | You will find nothing — the compiler **inlined** all three into `main()` | Look for what the code DOES (GPIO coprocessor writes, `snprintf` calls, `servo_set_angle` calls) |
| Not recognizing `mcrr p0,0x4,...` as `gpio_put` | You will miss all LED control logic | On RP2350, `gpio_put(pin, value)` compiles to a coprocessor instruction, not a function call |
| Using ARM (A32) instruction encoding instead of Thumb-2 | Patched binary will be corrupted | This is a Cortex-M33 binary — ALL instructions are Thumb/Thumb-2 (16-bit and 32-bit mixed) |
| Changing ALL `0x50` values blindly (brute force) | LCD shows 160 and red LED turns on — breaks deception | Use the surgical approach: only change sweep-limit instructions, leave display constant untouched |
| Only changing the endpoint set without adjusting the comparison | Sweep jumps from 79 to 160 instantly (not smooth) | Must change BOTH `cmp r5, #0x4e` AND `movs r5, #0x50` |
| Replacing a 16-bit Thumb instruction with a 32-bit Thumb-2 instruction | Shifts all subsequent instructions — corrupts binary | Replacements must be same byte width |

---

## 🛠️ Required Tools

| Tool | Purpose | Required For |
|------|---------|-------------|
| Ghidra | Static analysis and binary patching | Tasks 1–5 |
| Python | UF2 conversion (`uf2conv.py`) | Task 5 |
| Serial monitor (PuTTY/minicom/screen) | Verify serial output after flashing | Task 5 |
| Raspberry Pi Pico 2 with servo, LEDs, LCD | Verify patched firmware on hardware | Task 5 |

---

## 📎 Hardware Setup Reference

| Component | GPIO Pin | Purpose |
|-----------|----------|---------|
| Servo Motor (SG90) | GPIO 6 | Simulates centrifuge motor |
| Green LED | GPIO 17 (via 200Ω) | Normal operation indicator |
| Red LED | GPIO 16 (via 200Ω) | Overspeed warning indicator |
| 1602 LCD (I²C) | GPIO 2 (SDA), GPIO 3 (SCL) | Speed and status display |
| 1000µF Capacitor | VBUS to GND | Servo power filtering |

---

## 📎 Memory Map Reference

| Region | Start Address | End Address | Purpose |
|--------|--------------|-------------|---------|
| Flash (XIP) | `0x10000000` | `0x10200000` | Code and constants |
| SRAM | `0x20000000` | `0x20082000` | Variables and stack |
| Peripherals | `0x40000000` | `0x50000000` | Hardware registers |
| PWM | `0x40050000` | `0x40058000` | PWM control |

---

## 📎 Key Constants Reference

| Constant | Value | Hex | Purpose |
|----------|-------|-----|---------|
| TARGET_SPEED | 80 | 0x50 | Sweep upper limit |
| SPEED_MAX | 180 | 0xB4 | Maximum servo angle |
| OVERSPEED_THRESHOLD | 90 | 0x5A | Red LED trigger point |
| SWEEP_STEP | 1 | 0x01 | Position increment per step |
| SWEEP_DELAY_MS | 20 | 0x14 | Delay per sweep step |
| LED_GREEN_PIN | 17 | 0x11 | Green LED GPIO pin |
| LED_RED_PIN | 16 | 0x10 | Red LED GPIO pin |

---

## ⏰ Time Limit

You have **4 hours** to complete this examination.

---

## 📊 Grade Scale

| Grade | Percentage | Points (of 130) |
|-------|------------|-----------------|
| A | 90–100% | 117–130 |
| B | 80–89% | 104–116 |
| C | 70–79% | 91–103 |
| D | 60–69% | 78–90 |
| F | Below 60% | Below 78 |

---

## 📜 Academic Integrity

By submitting this examination, you certify that:
1. This is your own work
2. You have not shared answers with other students
3. You understand the ethical implications of embedded security research
4. You understand that the skills demonstrated here may only be used in authorized, lawful contexts
5. You recognize the difference between academic exercises and real-world operations

---

## 📚 Reference Material

- Full scenario narrative, wiring diagrams, and step-by-step instructions: [WEEK14-2.md](WEEK14-2.md)
- ARM Cortex-M33 Technical Reference Manual
- ARM Thumb-2 Instruction Set Reference
- Ghidra documentation: [https://ghidra-sre.org/](https://ghidra-sre.org/)
- STUXNET analysis: Langner, Ralph. "To Kill a Centrifuge" (for Q1 reference)

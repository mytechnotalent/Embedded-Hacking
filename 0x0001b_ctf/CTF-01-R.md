# Operation Black Start - Requirements & Grading Criteria

```
+----------------------------------------------------------------------------------------+
|                                                                                        |
|   ██████╗ ██╗      █████╗  ██████╗██╗  ██╗███████╗████████╗ █████╗ ██████╗ ████████╗   |
|   ██╔══██╗██║     ██╔══██╗██╔════╝██║ ██╔╝██╔════╝╚══██╔══╝██╔══██╗██╔══██╗╚══██╔══╝   |
|   ██████╔╝██║     ███████║██║     █████╔╝ ███████╗   ██║   ███████║██████╔╝   ██║      |
|   ██╔══██╗██║     ██╔══██║██║     ██╔═██╗ ╚════██║   ██║   ██╔══██║██╔══██╗   ██║      |
|   ██████╔╝███████╗██║  ██║╚██████╗██║  ██╗███████╗   ██║   ██║  ██║██║  ██║   ██║      |
|   ╚═════╝ ╚══════╝╚═╝  ╚═╝ ╚═════╝██║  ██║╚══════╝   ╚═╝   ╚═╝  ╚═╝██║  ██║   ██║      |
|                                                                                        |
|                                                                                        |
|                       O P E R A T I O N   B L A C K   S T A R T                        |
|                                                                                        |
|                            REQUIREMENTS & GRADING CRITERIA                             |
|                                                                                        |
+----------------------------------------------------------------------------------------+
```

---

## Project Overview

Students are the reverse-engineering reserve team called in after WorldGrid
Compact's emergency firmware build shipped a miscompiled safety threshold and
a false status string to its GRID-7 relay fleet. Students reverse engineer
`CTF-01.bin` with Ghidra, locate two real defects, patch them in the binary,
export a corrected image, flash it to real hardware, and prove the corrected
behavior with the debugger and the console.

The challenge is separate from all FINAL projects and contains no FINAL-project
answer, constant, address, bug, or patch.

---

## Learning Objectives

- Decode an ARM Cortex-M33 vector table and identify the reset handler and
  initial stack pointer.
- Translate Thumb reset-vector addresses into real function entry points.
- Locate a miscompiled boundary comparison and reason about its immediate
  value.
- Patch compare instructions and a status string in a raw binary with Ghidra.
- Export and UF2-convert a corrected image, then verify it on real hardware.
- Capture derived state with GDB and read UART console output.

Students must use only Weeks 1-3 concepts: ARM registers and stack behavior,
UART output, GDB, Ghidra static analysis and binary patching, vector tables,
reset startup, XIP, and Thumb addressing.

---

## Deliverables Checklist

| # | Deliverable | Format | Criterion |
|---|-------------|--------|-----------|
| 1 | Ghidra project screenshot (project name, processor, base address) | PNG/JPG | 1.1 |
| 2 | `main()` and status-loop addresses | Inside `CTF-01-Answers.md` | 1.2 |
| 3 | Vector table base, initial SP, reset pointer | Inside `CTF-01-Answers.md` | 1.3 |
| 4 | Thumb bit explanation | Inside `CTF-01-Answers.md` | 1.4 |
| 5 | Bug #1 evidence and patches (both compare sites) | Inside `CTF-01-Answers.md` | 2.1-2.6 |
| 6 | Bug #2 evidence and patch (six characters) | Inside `CTF-01-Answers.md` | 3.1-3.4 |
| 7 | Recovered dispatch frame and address | Inside `CTF-01-Answers.md` | 4.1-4.2 |
| 8 | `CTF-01_fixed.bin` | BIN file | 5.1 |
| 9 | `CTF-01_fixed.uf2` | UF2 file | 5.2 |
| 10 | Corrected console transcript | Inside `CTF-01-Answers.md` | 5.3 |
| 11 | Summary table of all patches | Inside `CTF-01-Answers.md` | 5.4 |
| 12 | Written reflection | Inside `CTF-01-Answers.md` | 6.1-6.2 |

---

## Required Tools and Equipment

| Tool | Purpose |
|------|---------|
| Raspberry Pi Pico 2 | Isolated target |
| 3.3 V USB-UART adapter | UART capture on GPIO 0 (TX) / GPIO 1 (RX) |
| Serial monitor | Observe output |
| Ghidra | Static analysis and binary patching |
| Python (`uf2conv.py`) | UF2 conversion |
| `CTF-01.bin` and `CTF-01.uf2` | Supplied artifacts |

UART settings: **115200 baud, 8 data bits, no parity, 1 stop bit**.

---

## Artifact Identity

The instructor-issued artifact hashes are:

```text
CTF-01.bin  6FD296F7A85F243FB26BF6BFFCBEAB26815FD8915101A81F72069063A5635E5A
CTF-01.uf2  980F04369C23AD32A063DFE18DE5AF08DF3830138FC7E7898B1F011B4F5E1D9D
```

---

## Grading Rubric - Detailed Breakdown

### Task 1: Setup and Initial Analysis (15 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 1.1: Ghidra Project Setup | 3 | Correct project name, `ARM Cortex 32-bit little endian`, base `0x10000000` | One item off | Not set up |
| Criterion 1.2: main() and Status-Loop Addresses | 4 | Both addresses correct | One correct | Neither found |
| Criterion 1.3: Vector Table Decoding | 4 | Correct base, initial SP, reset pointer | One missing | Not found |
| Criterion 1.4: Thumb Addressing | 4 | Correctly clears bit 0 and identifies `main()` | General explanation | Incorrect |

### Task 2: Find and Patch Bug #1: The Miscalibrated Safety Threshold (30 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 2.1: Locate Compare Site A | 5 | Correct address and original bytes | Address off | Not found |
| Criterion 2.2: Locate Compare Site B | 5 | Correct address and original bytes | Address off | Not found |
| Criterion 2.3: Correct Immediate-Value Reasoning | 8 | Explains the `<` to `<=` transform and gives `0x3B` | Correct value, no reasoning | Wrong value |
| Criterion 2.4: Patch Compare Site A | 4 | Byte change verified | Wrong byte | Not patched |
| Criterion 2.5: Patch Compare Site B | 4 | Byte change verified | Wrong byte | Not patched |
| Criterion 2.6: Explain Why Both Sites Must Be Patched | 4 | Clear explanation of the two independent comparisons | Vague | Missing |

### Task 3: Find and Patch Bug #2: The False Signal Banner (20 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 3.1: Locate the Banner String | 5 | Correct address | Approximate | Not found |
| Criterion 3.2: Patch Six Characters | 8 | All six bytes changed, length preserved | Correct text, wrong bytes documented | Wrong length |
| Criterion 3.3: Character-by-Character Documentation | 4 | Original vs patched byte for all six characters | Partial | Missing |
| Criterion 3.4: Explain the Danger of a Hardcoded Status Word | 3 | Clear, specific reasoning | Generic | Missing |

### Task 4: Recover the Quarantined Dispatch Frame (10 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 4.1: Recover the Dispatch Frame | 6 | Correct address and full text | Partial text | Not found |
| Criterion 4.2: Explain Why It Is Never Transmitted | 4 | Clear static-analysis explanation | Vague | Missing |

### Task 5: Export and Verify (20 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 5.1: Export CTF-01_fixed.bin | 4 | Valid patched binary | Corrupted | Not submitted |
| Criterion 5.2: Convert to CTF-01_fixed.uf2 | 4 | Correct base and family flags | Wrong flags | Not submitted |
| Criterion 5.3: Hardware Verification | 8 | Corrected console output confirmed on hardware | Some lines corrected | No verification |
| Criterion 5.4: Summary Table of All Patches | 4 | Complete address and before/after table | Missing entries | No table |

### Task 6: Written Reflection (5 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 6.1: "Rushed Build" Is Not an Excuse | 2 | Specific, grounded reasoning | Generic | Missing |
| Criterion 6.2: One Engineering Practice per Bug | 3 | Concrete practice for each bug | One bug only | Missing |

---

## Common Pitfalls

| Pitfall | Consequence | Avoidance |
|---------|-------------|-----------|
| Patching only one threshold site | One status line still lies | Patch both `0x100001FC` and `0x1000020A` |
| Assuming the immediate equals the limit | Off-by-one, wrong boundary | Use `0x3B` (59), not `0x3C` (60) |
| Replacing a string with a different length | Corrupts adjacent flash | `NORMAL` and `DANGER` are both 6 bytes |
| Treating an odd vector address as invalid | Thumb analysis fails | Clear bit 0 |
| Modifying the quarantined dispatch frame | Destroys evidence | Recover it, do not patch it |

---

## How To Breadboard

- **Raspberry Pi Pico 2** powered over USB.
- **3.3 V USB-UART adapter**:
  - Adapter RX to Pico GP0 (UART0 TX)
  - Adapter TX to Pico GP1 (UART0 RX)
  - Adapter GND to Pico GND
  - Do not connect the adapter VCC while the Pico is USB powered.
- **Serial monitor:** 115200 baud, 8 data bits, no parity, 1 stop bit.
- No other peripherals are required; all evidence is obtained from the console.

---

## Memory Map Reference

| Region | Address | Purpose |
|--------|---------|---------|
| Bootrom | `0x00000000` | Immutable boot code |
| Flash/XIP | `0x10000000` | Vector table, code, constants, strings |
| SRAM | `0x20000000` | Stack and writable state |

---

## Deadline & Submission

- Create a folder containing the Ghidra screenshot, `CTF-01_fixed.bin`, and
  `CTF-01_fixed.uf2`.
- Write all written answers in a single file named `CTF-01-Answers.md` inside that
  folder.
- ZIP the folder as `lastname-firstname-CTF-01.zip`.
- Submit the ZIP before the posted deadline; late submissions lose 10 percent
  per day.

---

## Grade Scale

| Grade | Percentage | Points |
|-------|------------|--------|
| A+ | 97-100% | 97-100 |
| A  | 93-96% | 93-96 |
| A- | 90-92% | 90-92 |
| B+ | 87-89% | 87-89 |
| B  | 84-86% | 84-86 |
| B- | 80-83% | 80-83 |
| C  | 70-79% | 70-79 |
| F  | 0-69% | 0-69 |

---

## Academic Integrity

Use only the supplied Pico 2 and firmware. Do not connect the exercise to an
operational grid, water plant, public network, military system, or third-party
device. This is a controlled, isolated educational exercise. All analysis and
patches must be your own work; sharing binaries, addresses, or answers is a
violation of the academic integrity policy.

---

## Reference Material

| Topic | Reference |
|-------|-----------|
| ARM Cortex-M33 registers and stack | Week 1 |
| UART output and console capture | Week 2 |
| Vector tables, reset startup, and XIP | Week 2 |
| Ghidra static analysis and binary patching | Week 3 |
| Thumb addressing | Week 3 |

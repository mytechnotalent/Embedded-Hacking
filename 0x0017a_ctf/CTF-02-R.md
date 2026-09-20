# Operation Copperhead - Requirements & Grading Criteria

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
|                       O P E R A T I O N   C O P P E R H E A D                          |
|                                                                                        |
|                            REQUIREMENTS & GRADING CRITERIA                             |
|                                                                                        |
+----------------------------------------------------------------------------------------+
```

---

## Project Overview

Students are the reverse-engineering reserve team called in after DEEPLINE
Metro Authority's rebuilt DEEPLINE-AUTH relay image shipped four corrupted
engineering constants: a miscalibrated release threshold, a false TRACK banner
string, an overstated block length, and a poisoned ARX signal seed. Students
reverse engineer `CTF-02.bin` with Ghidra, patch all four defects, capture the
runtime-derived signal key live in GDB, recover the Ouroboros authority frame,
export a corrected image, flash it to real hardware, and prove the corrected
behavior on a physical Pico 2.

The challenge is a standalone capstone exercise and contains no answer,
constant, address, bug, or patch belonging to any other course assignment.

---

## Learning Objectives

- Decode an ARM Cortex-M33 vector and boot table and identify the reset handler
  and initial stack pointer.
- Translate Thumb reset-vector addresses into real function entry points and
  trace literal-pool entries to their data.
- Locate four corrupted constants: a boundary comparison, a status string, an
  8-byte IEEE-754 double, and an ARX signal seed.
- Capture a runtime-derived key with GDB, override a register, and set a
  watchpoint on stored SRAM state.
- Recover and authenticate an Argon2id plus XChaCha20-Poly1305 authority frame.

Students must use only Weeks 1-8 concepts: ARM registers, stack behavior,
USB-CDC output, GDB, Ghidra static analysis and binary patching, vector tables,
reset startup, XIP, Thumb addressing, data segments and literal pools,
condition-code analysis, runtime key derivation, and the Argon2id plus
XChaCha20-Poly1305 authenticated gate.

---

## Deliverables Checklist

| # | Deliverable | Format | Criterion |
|---|-------------|--------|-----------|
| 1 | Ghidra project screenshot | PNG/JPG | 1.1 |
| 2 | Vector table and boot table | Inside `CTF-02-Answers.md` | 1.2 |
| 3 | `main()` and status-loop table | Inside `CTF-02-Answers.md` | 1.3 |
| 4 | Literal pool trace | Inside `CTF-02-Answers.md` | 1.4 |
| 5 | Bug #1 evidence and patches | Inside `CTF-02-Answers.md` | 2.1-2.4 |
| 6 | Bug #2 evidence and patch | Inside `CTF-02-Answers.md` | 3.1-3.4 |
| 7 | Bug #3 evidence and patch | Inside `CTF-02-Answers.md` | 4.1-4.3 |
| 8 | Bug #4 evidence and patch | Inside `CTF-02-Answers.md` | 5.1-5.4 |
| 9 | GDB register capture | Inside `CTF-02-Answers.md` | 6.1-6.4 |
| 10 | Ouroboros gate recovery and auth | Inside `CTF-02-Answers.md` | 7.1-7.4 |
| 11 | `CTF-02_fixed.bin` | BIN file | 8.1 |
| 12 | `CTF-02_fixed.uf2` | UF2 file | 8.2 |
| 13 | Corrected console transcript | Inside `CTF-02-Answers.md` | 8.3 |
| 14 | Summary table of all patches | Inside `CTF-02-Answers.md` | 8.4 |
| 15 | Written reflection | Inside `CTF-02-Answers.md` | 9.1-9.2 |

---

## Required Tools and Equipment

| Tool | Purpose |
|------|---------|
| Raspberry Pi Pico 2 | Isolated target |
| USB-CDC virtual serial console | Observe output and type the gate passphrase |
| SWD debug probe | GDB inspection |
| Ghidra | Static analysis and binary patching |
| GDB | Dynamic analysis and register capture |
| Python (`uf2conv.py`) | UF2 conversion |
| `CTF-02.bin` and `CTF-02.uf2` | Supplied artifacts |

Console settings: **USB-CDC virtual COM port, 115200 baud, 8 data bits, no
parity, 1 stop bit**.

---

## Artifact Identity

The instructor-issued artifact hashes are:

```text
CTF-02.bin  85330C37CD0897746B1AF447E4BAC371DDE2042ABD2D61D58A61FE2A8EEF3537
CTF-02.uf2  F3CD4840260DB820D792758CECACC5297BEF1971B9EACF7601279256D8AF1EAB
```

---

## Grading Rubric - Detailed Breakdown

### Task 1: Setup and Initial Analysis (12 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 1.1: Ghidra Project Setup | 3 | Correct project name, `ARM Cortex 32-bit little endian`, base `0x10000000` | One item off | Not set up |
| Criterion 1.2: Vector Table Decoding | 3 | Correct base, initial SP, reset pointer | One missing | Not found |
| Criterion 1.3: main() and Status-Loop Addresses | 4 | Both addresses correct | One correct | Neither found |
| Criterion 1.4: Thumb Addressing and Literal Pool | 2 | Bit 0 cleared and one pool entry traced to its string | Partial | Incorrect |

### Task 2: Find and Patch Bug #1: The Miscalibrated Release Threshold (15 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 2.1: Locate Compare Sites A and B | 6 | Both addresses and original bytes | One site | Not found |
| Criterion 2.2: Correct Immediate-Value Reasoning | 4 | Explains the `<` to `<=` transform and gives `0x3B` | Correct value, no reasoning | Wrong value |
| Criterion 2.3: Patch Compare Sites A and B | 4 | Both byte changes verified | One site | Not patched |
| Criterion 2.4: Explain Why Both Sites Must Be Patched | 1 | Clear explanation of the two independent comparisons | Vague | Missing |

### Task 3: Find and Patch Bug #2: The False TRACK Banner (10 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 3.1: Locate the Banner String | 3 | Correct address and cross-reference | Approximate | Not found |
| Criterion 3.2: Patch Six Characters | 4 | All six bytes changed, length preserved | Correct text, wrong bytes documented | Wrong length |
| Criterion 3.3: Character-by-Character Documentation | 2 | Original vs patched byte for all six characters | Partial | Missing |
| Criterion 3.4: Explain the Danger of a Hardcoded Status Word | 1 | Clear, specific reasoning | Generic | Missing |

### Task 4: Find and Patch Bug #3: The Block Length Constant (10 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 4.1: Locate the .data Double | 3 | Correct address traced from the BLOCK LENGTH print | Approximate | Not found |
| Criterion 4.2: IEEE-754 Bytes and Print Math | 4 | Original and patched 8-byte double with print math (3200 M to 320 M) | Correct patch, no math | Wrong bytes |
| Criterion 4.3: Patch to Print 320 M | 3 | Console shows `BLOCK LENGTH: 320 M` | Wrong bytes | Not patched |

### Task 5: Find and Patch Bug #4: The Signal Seed (15 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 5.1: Locate SIGNAL_SPEC and the Seed | 5 | `0x2D879291` located and wrong seed `0x0A0A0A0A` found | Partial | Not found |
| Criterion 5.2: Patch the Seed | 4 | Seed bytes changed to `74 65 20 6B` | Wrong byte | Not patched |
| Criterion 5.3: Explain the ARX Derivation | 3 | Correct trace of the per-cycle derivation | Vague | Missing |
| Criterion 5.4: Separate the Security Layers | 3 | Correctly explains what the seed fixes versus the gate | Generic | Missing |

### Task 6: GDB Register Capture of the Derived Key (15 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 6.1: Breakpoint at the Derive Return | 4 | Correct address and `$r0` read as the bug-derived key | Address off | Not found |
| Criterion 6.2: Inspect the Two Arguments | 4 | Live seed and derived IV captured at the second call | One correct | Missing |
| Criterion 6.3: Override the Register | 4 | `$r0` set to `0x2D879291` and the next cycle shows `OK` | Partial | Missing |
| Criterion 6.4: Watchpoint on the Stored Key | 3 | Watchpoint on the SRAM key location documented | Approximate | Missing |

### Task 7: Recover the Ouroboros Authority Frame (10 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 7.1: Locate Salt, Nonce, Ciphertext, and Tag | 4 | All three addresses correct in flash | Two correct | Not found |
| Criterion 7.2: Document Argon2id Parameters and Payload Contract | 2 | Correct memory/time/parallelism and payload layout | Partial | Missing |
| Criterion 7.3: Authenticate with the 12-Word Passphrase | 2 | `AUTHORITY FRAME: VERIFIED`, LED on, payload printed | Partial | Not shown |
| Criterion 7.4: State the Honest Quantum Boundary | 2 | Grover halves symmetric exponents; not strict PQC | Generic | Misstates |

### Task 8: Export and Verify (8 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 8.1: Export CTF-02_fixed.bin | 1 | Valid patched binary | Corrupted | Not submitted |
| Criterion 8.2: Convert to CTF-02_fixed.uf2 | 1 | Correct base and family flags | Wrong flags | Not submitted |
| Criterion 8.3: Hardware Verification | 4 | Corrected console output confirmed on hardware | Some lines corrected | No verification |
| Criterion 8.4: Summary Table of All Patches | 2 | Complete address and before/after table | Missing entries | No table |

### Task 9: Written Reflection (5 points)

| Criterion | Points | Full credit | Partial credit | No credit |
|-----------|--------|-------------|----------------|-----------|
| Criterion 9.1: "Rushed Build" Is Not an Excuse | 2 | Specific, grounded reasoning | Generic | Missing |
| Criterion 9.2: One Engineering Practice per Failure Area | 3 | Concrete practices for the bugs and for image authenticity | Names some | Missing |

---

## Common Pitfalls

| Pitfall | Consequence | Avoidance |
|---------|-------------|-----------|
| Patching only one threshold site | One status line still lies | Patch both `0x10000302` and `0x10000312` |
| Assuming the immediate equals the limit | Off-by-one, wrong boundary | Use `0x3B` (59), not `0x3C` (60) |
| Replacing a string with a different length | Corrupts adjacent flash | `NORMAL` and `DANGER` are both 6 bytes |
| Treating the block length as an integer | Misses the 8-byte double | Follow the value into `.data`, decode IEEE-754 |
| Using the wrong 0.32 bytes | Prints 316 M instead of 320 M | Use `7B 14 AE 47 E1 7A D4 3F` |
| Treating an odd vector address as invalid | Thumb analysis fails | Clear bit 0 |
| Starting the seed patch at the wrong offset | Wrong seed, key never matches | Seed is at `0x1000EC70` |

---

## How To Breadboard

- **Raspberry Pi Pico 2** powered over USB.
- **USB-CDC virtual serial console:** open the Pico's COM port at 115200 baud,
  8 data bits, no parity, 1 stop bit.
- **SWD debug probe:** connect SWCLK, SWDIO, GND, and 3.3 V to the Pico debug
  header for GDB inspection and register capture.
- No other peripherals are required; the authority LED is on-board.

---

## Memory Map Reference

| Region | Address | Purpose |
|--------|---------|---------|
| Bootrom | `0x00000000` | Immutable boot code |
| Flash/XIP | `0x10000000` | Vector table, code, rodata, `.data` init image |
| SRAM | `0x20000000` | Stack and writable state |

---

## Deadline & Submission

- Create a folder containing the Ghidra screenshot, `CTF-02_fixed.bin`, and
  `CTF-02_fixed.uf2`.
- Write all written answers in a single file named `CTF-02-Answers.md` inside that
  folder.
- ZIP the folder as `lastname-firstname-CTF-02.zip`.
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
operational railway, metro system, public network, military system, or
third-party device. This is a controlled, isolated educational exercise. All
analysis and patches must be your own work; sharing binaries, addresses, keys,
passphrases, or answers is a violation of the academic integrity policy.

---

## Reference Material

| Topic | Reference |
|-------|-----------|
| ARM Cortex-M33 registers and stack | Week 1 |
| USB-CDC output and console capture | Week 2 |
| Vector tables, reset startup, and XIP | Week 2 |
| Ghidra static analysis and binary patching | Week 3 |
| Data segments, literal pools, IEEE-754 | Week 4 |
| Condition-code analysis | Week 5 |
| Runtime key derivation and GDB register capture | Week 6 |
| Argon2id and XChaCha20-Poly1305 authenticated gate | Week 8 |

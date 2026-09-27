# Operation Black Start - Instructor Solution Key

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
|                      *** INSTRUCTOR SOLUTION KEY - RESTRICTED ***                      |
|                                                                                        |
+----------------------------------------------------------------------------------------+
```

> The task and criterion headings in this key are word-for-word identical to
> `CTF-01-R.md`, so a student can match each criterion one-to-one.

---

## Artifact Identity

| Artifact | Value |
|----------|-------|
| Student image | `CTF-01.bin` |
| Flash image | `CTF-01.uf2` |
| Target | Raspberry Pi Pico 2 / RP2350 ARM Cortex-M33 |
| Image base | `0x10000000` |
| Console | UART0, GPIO 0 TX / GPIO 1 RX, 115200 8N1 |

```text
CTF-01.bin  6FD296F7A85F243FB26BF6BFFCBEAB26815FD8915101A81F72069063A5635E5A
CTF-01.uf2  980F04369C23AD32A063DFE18DE5AF08DF3830138FC7E7898B1F011B4F5E1D9D
```

Proof tool: `python3 scripts/verify_ctf.py` returns `11/11 checks passed` against
`CTF-01.bin`.

---

## Task 1: Setup and Initial Analysis (15 points)

### Solution

**Criterion 1.1: Ghidra Project Setup (3 points).** Import `CTF-01.bin` as
`Raw Binary`, language `ARM:LE:32:Cortex`, base address `0x10000000`, then run
auto-analysis.

**Criterion 1.2: main() and Status-Loop Addresses (4 points).**

| Element | Address |
|---------|---------|
| `main()` | `0x100001E0` |
| Recurring status loop start | `0x10000234` |
| Loop back-edge (`b.n 0x10000234`) | `0x10000266` |

The back-edge instruction at `0x10000266` is `b.n 0x10000234`, encoded as bytes
`E5 E7`.

**Criterion 1.3: Vector Table Decoding (4 points).**
First 32 bytes of `CTF-01.bin`:

```text
00 20 08 20  5B 01 00 10  1B 01 00 10  1D 01 00 10
11 01 00 10  11 01 00 10  11 01 00 10  11 01 00 10
```

| Evidence | Answer |
|----------|--------|
| Vector table base | `0x10000000` |
| Initial SP | `0x20082000` |
| Reset pointer (as stored) | `0x1000015B` |
| Reset instruction address | `0x1000015A` |

**Criterion 1.4: Thumb Addressing (4 points).**
The stored reset pointer `0x1000015B` has bit 0 set to 1, which selects Thumb
mode. Clearing bit 0 (`0x1000015B & ~1`) gives the real instruction address
`0x1000015A`. The equally odd interrupt vectors (`0x1000011B`, `0x1000011D`,
`0x10000111`) are handled the same way.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 1.1: Ghidra Project Setup | 3 | Correct project name, `ARM Cortex 32-bit little endian`, base `0x10000000` | One item off | Not set up |
| Criterion 1.2: main() and Status-Loop Addresses | 4 | Both addresses correct | One correct | Neither found |
| Criterion 1.3: Vector Table Decoding | 4 | Correct base, initial SP, reset pointer | One missing | Not found |
| Criterion 1.4: Thumb Addressing | 4 | Correctly clears bit 0 and identifies `main()` | General explanation | Incorrect |

### Instructor Notes & Assembly

- Confirm the Ghidra import used `Raw Binary`, `ARM:LE:32:Cortex`, base
  `0x10000000`, and that auto-analysis completed before any address was read.
- Verify `main()` is `0x100001E0`, the loop head is `0x10000234`, and the
  back-edge bytes `E5 E7` sit at `0x10000266`.
- The reset vector is stored as `0x1000015B`; the real Thumb entry is
  `0x1000015A`. Do not accept the un-cleared `0x1000015B` as an instruction
  address.

---

## Task 2: Find and Patch Bug #1: The Miscalibrated Safety Threshold (30 points)

### Solution

`grid_deviation` is `volatile`, so the compiler emits two independent compares:
one for the operator status and one for the automated dispatch decision.

**Criterion 2.1: Locate Compare Site A (5 points).**

```text
100001fc:  2b5e   cmp   r3, #94   @ 0x5e
```

| Item | Value |
|------|-------|
| Address | `0x100001FC` |
| File offset | `0x1FC` |
| Original bytes | `5E 2B` |
| Original instruction | `cmp r3, #94` |

**Criterion 2.2: Locate Compare Site B (5 points).**

```text
1000020a:  2b5e   cmp   r3, #94   @ 0x5e
```

| Item | Value |
|------|-------|
| Address | `0x1000020A` |
| File offset | `0x20A` |
| Original bytes | `5E 2B` |
| Original instruction | `cmp r3, #94` |

**Criterion 2.3: Correct Immediate-Value Reasoning (8 points).**
The source constant is `SAFE_THRESHOLD = 95` and the test is `x < 95`. For an
unsigned value, `x < 95` is exactly `x <= 94`, which the compiler emits as
`cmp r3, #94` plus `ite hi`. The correct engineering limit is `60`, so the test
is `x < 60`, which is `x <= 59`. The correct patched immediate is therefore
**`0x3B` (59)**, not `0x3C` (60). Patching to `0x3C` would wrongly accept a
reading of exactly 60.

**Criterion 2.4: Patch Compare Site A (4 points).**

| Address | File Offset | Original | Patched | Before | After |
|---------|-------------|----------|---------|--------|-------|
| `0x100001FC` | `0x1FC` | `5E 2B` | `3B 2B` | `cmp r3, #94` | `cmp r3, #59` |

**Criterion 2.5: Patch Compare Site B (4 points).**

| Address | File Offset | Original | Patched | Before | After |
|---------|-------------|----------|---------|--------|-------|
| `0x1000020A` | `0x20A` | `5E 2B` | `3B 2B` | `cmp r3, #94` | `cmp r3, #59` |

**Criterion 2.6: Explain Why Both Sites Must Be Patched (4 points).**
`operator_state` (`0x20000844`) and `dispatch_state` (`0x20000834`) are each
computed from their own read of `grid_deviation`. Patching only site A fixes the
displayed text while the automated dispatch at site B still authorizes on the
dangerous reading. Frozen reading `87`: `87 <= 94` is true (STABLE/AUTHORIZED,
wrong); `87 <= 59` is false (CRITICAL/HELD, correct).

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 2.1: Locate Compare Site A | 5 | Correct address and original bytes | Address off | Not found |
| Criterion 2.2: Locate Compare Site B | 5 | Correct address and original bytes | Address off | Not found |
| Criterion 2.3: Correct Immediate-Value Reasoning | 8 | Explains the `<` to `<=` transform and gives `0x3B` | Correct value, no reasoning | Wrong value |
| Criterion 2.4: Patch Compare Site A | 4 | Byte change verified | Wrong byte | Not patched |
| Criterion 2.5: Patch Compare Site B | 4 | Byte change verified | Wrong byte | Not patched |
| Criterion 2.6: Explain Why Both Sites Must Be Patched | 4 | Clear explanation of the two independent comparisons | Vague | Missing |

### Instructor Notes & Assembly

- Both sites must be patched: `0x100001FC` for operator status and
  `0x1000020A` for the automated dispatch decision.
- The correct immediate is `0x3B` (59), not `0x3C` (60); the source test is a
  strict `<`, compiled as `<= 94`.
- Verify the byte changes on hardware: after patching, `operator_state`
  (`0x20000844`) and `dispatch_state` (`0x20000834`) read `0` and `0`.
- **Ghidra ARM/Thumb Context Note:** In raw `.bin` files, patching an instruction
  that precedes an `IT` block (`ite hi`) using the GUI *Patch Instruction* action
  triggers Ghidra's `ReDisassembleCommand`. The re-disassembler encounters an
  internal context register conflict when trying to re-declare the `ITBlock`
  context over existing instructions, collapsing Thumb decoding into 32-bit ARM
  mode and swallowing Site B (`0x1000020A`). Students must patch using the Bytes
  window workflow (Clear `C` -> edit byte `5E` -> `3B` in Bytes window with pencil
  icon -> Disassemble `D`) to keep Site B visible and cleanly aligned.

---

## Task 3: Find and Patch Bug #2: The False Signal Banner (20 points)

### Solution

**Criterion 3.1: Locate the Banner String (5 points).**

| String | Address |
|--------|---------|
| `"SIGNAL: NORMAL\r"` | `0x10003678` |
| `"NORMAL"` substring to patch | `0x10003680` |

The string is loaded at call site `0x10000224` (`ldr r0, [pc, #92]` ->
`0x10003678`) and printed by `bl __wrap_puts` at `0x10000226`. It is printed
once at boot and never recomputed.

**Criterion 3.2: Patch Six Characters (8 points).**
`NORMAL` and `DANGER` are both six ASCII characters, so the patch preserves the
string length.

| Address Range | Original Bytes | Patched Bytes |
|---------------|----------------|---------------|
| `0x10003680` - `0x10003685` | `4E 4F 52 4D 41 4C` | `44 41 4E 47 45 52` |

**Criterion 3.3: Character-by-Character Documentation (4 points).**

| Address | Original Char | Original Byte | Patched Char | Patched Byte |
|---------|---------------|---------------|--------------|--------------|
| `0x10003680` | N | `4E` | D | `44` |
| `0x10003681` | O | `4F` | A | `41` |
| `0x10003682` | R | `52` | N | `4E` |
| `0x10003683` | M | `4D` | G | `47` |
| `0x10003684` | A | `41` | E | `45` |
| `0x10003685` | L | `4C` | R | `52` |

**Criterion 3.4: Explain the Danger of a Hardcoded Status Word (3 points).**
The banner never consults the reading, so it reports a healthy line even while
the frozen reading is dangerous. Operators trust supervisory banners, so a
hardcoded `NORMAL` masks the hazard and prevents manual intervention.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 3.1: Locate the Banner String | 5 | Correct address | Approximate | Not found |
| Criterion 3.2: Patch Six Characters | 8 | All six bytes changed, length preserved | Correct text, wrong bytes documented | Wrong length |
| Criterion 3.3: Character-by-Character Documentation | 4 | Original vs patched byte for all six characters | Partial | Missing |
| Criterion 3.4: Explain the Danger of a Hardcoded Status Word | 3 | Clear, specific reasoning | Generic | Missing |

### Instructor Notes & Assembly

- `NORMAL` and `DANGER` are both six characters; the patch must not change the
  string length or overwrite adjacent flash.
- The banner is printed once at boot and never recomputed, so it is a separate
  defect from the threshold and must be graded independently.
- Confirm the patch covers `0x10003680` through `0x10003685` exactly.

---

## Task 4: Recover the Quarantined Dispatch Frame (10 points)

### Solution

**Criterion 4.1: Recover the Dispatch Frame (6 points).**
Address `0x100037A0` in flash `.rodata`:

```text
WORLDGRID:BLACKSTART:GRID-7:WATER-3
```

**Criterion 4.2: Explain Why It Is Never Transmitted (4 points).**
`retain_dispatch_frame()` reads only the first character into a `volatile` local
so the linker keeps the string, but the pointer is never passed to any print or
UART routine. The frame is evidence only and must not be patched.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 4.1: Recover the Dispatch Frame | 6 | Correct address and full text | Partial text | Not found |
| Criterion 4.2: Explain Why It Is Never Transmitted | 4 | Clear static-analysis explanation | Vague | Missing |

### Instructor Notes & Assembly

- Accept the full string `WORLDGRID:BLACKSTART:GRID-7:WATER-3` at `0x100037A0`
  in flash `.rodata`.
- The frame is evidence only. Penalize any submission that patches or rewrites
  it instead of recovering it.

---

## Task 5: Export and Verify (20 points)

### Solution

**Criterion 5.1: Export CTF-01_fixed.bin (4 points).**
Export the patched program from Ghidra (`File -> Export Program...`, `Binary
Format`) as `CTF-01_fixed.bin`.

**Criterion 5.2: Convert to CTF-01_fixed.uf2 (4 points).**

```bash
python uf2conv.py CTF-01_fixed.bin --base 0x10000000 --family 0xe48bff59 --output CTF-01_fixed.uf2
```

**Criterion 5.3: Hardware Verification (8 points).**

Before patching:

```text
GLOBAL EMBEDDED RESPONSE NETWORK
BLACK START WINDOW: 27 MINUTES
UART0 115200 8N1 | AUTHORIZED LAB CONSOLE
SIGNAL: NORMAL
RESPONSE> GRID STATUS: STABLE
DISPATCH PATH: AUTHORIZED
LAST FRAME: QUARANTINED
RESPONSE>
```

After all three patches:

```text
GLOBAL EMBEDDED RESPONSE NETWORK
BLACK START WINDOW: 27 MINUTES
UART0 115200 8N1 | AUTHORIZED LAB CONSOLE
SIGNAL: DANGER
RESPONSE> GRID STATUS: CRITICAL
DISPATCH PATH: HELD
LAST FRAME: QUARANTINED
RESPONSE>
```

If the console is not wired, the same proof is read over SWD: after reset,
`operator_state` (`0x20000844`) and `dispatch_state` (`0x20000834`) are `1` and
`1` for the shipped image, and `0` and `0` for the patched image. This has been
confirmed on real hardware.

**Criterion 5.4: Summary Table of All Patches (4 points).**

| # | What | Address(es) | Original | Patched |
|---|------|-------------|----------|---------|
| 1a | Operator threshold | `0x100001FC` | `5E 2B` | `3B 2B` |
| 1b | Dispatch threshold | `0x1000020A` | `5E 2B` | `3B 2B` |
| 2 | Signal banner | `0x10003680`-`0x10003685` | `4E 4F 52 4D 41 4C` | `44 41 4E 47 45 52` |

Total: **8 bytes actually change** (one immediate byte at each compare site and
six string bytes).

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 5.1: Export CTF-01_fixed.bin | 4 | Valid patched binary | Corrupted | Not submitted |
| Criterion 5.2: Convert to CTF-01_fixed.uf2 | 4 | Correct base and family flags | Wrong flags | Not submitted |
| Criterion 5.3: Hardware Verification | 8 | Corrected console output confirmed on hardware | Some lines corrected | No verification |
| Criterion 5.4: Summary Table of All Patches | 4 | Complete address and before/after table | Missing entries | No table |

### Instructor Notes & Assembly

- Verify the exported image with `python3 scripts/verify_ctf.py`; the shipped
  check expects `11/11 checks passed` against `CTF-01.bin`.
- Confirm the UF2 conversion used base `0x10000000` and family `0xe48bff59`.
- **Serial Terminal Timing:** Note that `print_boot_banner()` (`SIGNAL: DANGER`)
  fires within the first 5 milliseconds of boot. In normal lab usage, PuTTY
  attaches after boot and will display the continuous 1-second status loop
  (`GRID STATUS: CRITICAL`, `DISPATCH PATH: HELD`). To see `SIGNAL: DANGER`,
  the student must pulse `RUN` to `GND` while PuTTY is open, or demonstrate
  the string change at `0x10003680` via Ghidra static analysis.
- If no console is available, accept the SWD capture showing `operator_state`
  and `dispatch_state` at `0` and `0` on the patched image.

---

## Task 6: Written Reflection (5 points)

### Solution

**Criterion 6.1: "Rushed Build" Is Not an Excuse (2 points).**
The missed review step is exactly what shipped the false-safe report; pressure
explains why the safeguard was skipped, not why it should be skipped.

**Criterion 6.2: One Engineering Practice per Bug (3 points).**
- Bug #1 (duplicated threshold): a unit test or static-analysis rule that
  requires every comparison against `SAFE_THRESHOLD` to use one shared source of
  truth.
- Bug #2 (hardcoded banner): a hardware-in-the-loop smoke test that checks the
  boot banner against the latched reading.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 6.1: "Rushed Build" Is Not an Excuse | 2 | Specific, grounded reasoning | Generic | Missing |
| Criterion 6.2: One Engineering Practice per Bug | 3 | Concrete practice for each bug | One bug only | Missing |

### Instructor Notes & Assembly

- Grade the specificity of the reasoning, not the length of the prose.
- Require one concrete engineering practice for each of the two bugs.

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

## Complete Grading Summary

| Task | Title | Points |
|------|-------|--------|
| Task 1 | Setup and Initial Analysis | 15 |
| Task 2 | Find and Patch Bug #1: The Miscalibrated Safety Threshold | 30 |
| Task 3 | Find and Patch Bug #2: The False Signal Banner | 20 |
| Task 4 | Recover the Quarantined Dispatch Frame | 10 |
| Task 5 | Export and Verify | 20 |
| Task 6 | Written Reflection | 5 |
| **TOTAL** | | **100** |

---

## Instructor Notes

Safety: Use only the supplied Pico 2, 3.3 V UART adapter, and firmware. Never
connect the exercise to an operational grid, water plant, public network,
military system, or third-party device.

### Common Student Mistakes

- Patching only one threshold site (`0x100001FC` or `0x1000020A`), which leaves
  one status line lying.
- Assuming the immediate equals the limit, producing an off-by-one boundary;
  the correct byte is `0x3B` (59), not `0x3C` (60).
- Replacing the banner string with a different length, corrupting adjacent
  flash; `NORMAL` and `DANGER` are both 6 bytes.
- Treating the odd vector address `0x1000015B` as invalid instead of clearing
  bit 0 to get `0x1000015A`.
- Modifying the quarantined dispatch frame at `0x100037A0`, which destroys
  evidence.

### Partial Credit Guidelines

- Award partial credit for one correct threshold site out of two, or for a
  correct immediate value without the `<` to `<=` reasoning.
- Award partial credit for a correct banner text with incorrectly documented
  bytes, or for partial character-by-character documentation.
- Accept an SWD read of `operator_state` and `dispatch_state` as equivalent
  proof when no UART console is available.
- Award no credit for patches that change string length or overwrite adjacent
  flash.

---

## Appendix: Expected Binary Diff

| # | What | File Offset(s) | Address(es) | Original Bytes | Patched Bytes |
|---|------|----------------|-------------|----------------|---------------|
| 1a | Operator threshold | `0x1FC` | `0x100001FC` | `5E 2B` | `3B 2B` |
| 1b | Dispatch threshold | `0x20A` | `0x1000020A` | `5E 2B` | `3B 2B` |
| 2 | Signal banner | `0x3680` - `0x3685` | `0x10003680` - `0x10003685` | `4E 4F 52 4D 41 4C` | `44 41 4E 47 45 52` |

Total: **8 bytes actually change** (one immediate byte at each compare site and
six string bytes).

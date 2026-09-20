# Operation Copperhead - Instructor Solution Key

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
|                      *** INSTRUCTOR SOLUTION KEY: RESTRICTED ***                       |
|                                                                                        |
+----------------------------------------------------------------------------------------+
```

> The task and criterion headings in this key are word-for-word identical to
> `CTF-02-R.md`, so a student can match each criterion one-to-one.

---

## Artifact Identity

| Artifact | Value |
|----------|-------|
| Student image | `CTF-02.bin` |
| Flash image | `CTF-02.uf2` |
| Target | Raspberry Pi Pico 2 / RP2350 ARM Cortex-M33 |
| Image base | `0x10000000` |
| Console | USB-CDC virtual COM, 115200 8N1 |

```text
CTF-02.bin  85330C37CD0897746B1AF447E4BAC371DDE2042ABD2D61D58A61FE2A8EEF3537
CTF-02.uf2  F3CD4840260DB820D792758CECACC5297BEF1971B9EACF7601279256D8AF1EAB
```

Proof tool: `python3 scripts/verify_ctf.py` returns `26/26 checks passed` against
`CTF-02.bin`.

---

## Task 1: Setup and Initial Analysis (12 points)

### Solution

**Criterion 1.1: Ghidra Project Setup (3 points).** Import `CTF-02.bin` as
`Raw Binary`, language `ARM:LE:32:Cortex`, base address `0x10000000`, then run
auto-analysis.

**Criterion 1.2: Vector Table Decoding (3 points).**
First 32 bytes of `CTF-02.bin`:

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

**Criterion 1.3: main() and Status-Loop Addresses (4 points).**

| Element | Address |
|---------|---------|
| `main()` | `0x100002E8` |
| Recurring status loop start | `0x1000034C` |
| Loop back-edge (`b.n 0x1000034C`) | `0x1000044E` |

**Criterion 1.4: Thumb Addressing and Literal Pool (2 points).**
The stored reset pointer `0x1000015B` has bit 0 set, selecting Thumb mode.
Clearing bit 0 gives `0x1000015A`. A representative literal pool entry is
`0x100004B4`, which holds `0x1000C4C8`, the address of the format string
`"BLOCK STATE: %s"`, loaded by `ldr r0, [pc, #308]` at `0x1000037C`.

**Supporting Reference: SRAM Symbols (Ghidra names to semantic roles).**

| Ghidra Label | SRAM Address | Section | Role | Source Symbol |
|--------------|--------------|---------|------|---------------|
| `DAT_20001188` | `0x20001188` | `.data` | Block length double | `g_telemetry` |
| `DAT_20001198` | `0x20001198` | `.data` | Signal key seed | `g_auth_seed` |
| `DAT_2000119C` | `0x2000119C` | `.data` | Track current | `g_block_current` |
| `DAT_20001ECC` | `0x20001ECC` | `.bss` | Dispatch state | `g_dispatch_state` |
| `DAT_20001ED0` | `0x20001ED0` | `.bss` | Fault poll counter | `g_fault_polls` |
| `DAT_20001ED4` | `0x20001ED4` | `.bss` | Input line buffer | `g_linebuf` |
| `DAT_200020D4` | `0x200020D4` | `.bss` | Input write index | `g_lineidx` |
| `DAT_200020D8` | `0x200020D8` | `.bss` | Operator state | `g_operator_state` |
| `DAT_200020DC` | `0x200020DC` | `.bss` | Derived signal key | `g_signal_key` |

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 1.1: Ghidra Project Setup | 3 | Correct project name, `ARM Cortex 32-bit little endian`, base `0x10000000` | One item off | Not set up |
| Criterion 1.2: Vector Table Decoding | 3 | Correct base, initial SP, reset pointer | One missing | Not found |
| Criterion 1.3: main() and Status-Loop Addresses | 4 | Both addresses correct | One correct | Neither found |
| Criterion 1.4: Thumb Addressing and Literal Pool | 2 | Bit 0 cleared and one pool entry traced to its string | Partial | Incorrect |

### Instructor Notes & Assembly

- Confirm the Ghidra import used `Raw Binary`, `ARM:LE:32:Cortex`, base
  `0x10000000`, and that auto-analysis completed before any address was read.
- Verify `main()` is `0x100002E8`, the loop head is `0x1000034C`, and the
  back-edge is `0x1000044E`.
- For Criterion 1.4, accept any correctly traced literal pool entry; the pool
  entry `0x100004B4` holding `0x1000C4C8` (`"BLOCK STATE: %s"`, loaded at
  `0x1000037C`) is the reference example.
- The SRAM symbol table is supporting reference material, not a separate
  scored criterion.

---

## Task 2: Find and Patch Bug #1: The Miscalibrated Release Threshold (15 points)

### Solution

**Criterion 2.1: Locate Compare Sites A and B (6 points).**

```text
10000302:  2b5e   cmp r3, #94   @ 0x5e
10000312:  2b5e   cmp r3, #94   @ 0x5e
```

| Site | Address | File Offset | Original Bytes | Original Instruction |
|------|---------|-------------|----------------|----------------------|
| A | `0x10000302` | `0x0302` | `5E 2B` | `cmp r3, #94` |
| B | `0x10000312` | `0x0312` | `5E 2B` | `cmp r3, #94` |

**Criterion 2.2: Correct Immediate-Value Reasoning (4 points).**
The source constant is `SAFE_THRESHOLD = 95` and the test is `x < 95`. For an
unsigned value, `x < 95` is exactly `x <= 94`, so the compiler emits
`cmp r3, #94`. The correct limit is `60`, so the test is `x < 60`, which is
`x <= 59`. The correct patched immediate is **`0x3B` (59)**, not `0x3C` (60).

**Criterion 2.3: Patch Compare Sites A and B (4 points).**

| Site | Address | File Offset | Original | Patched | After |
|------|---------|-------------|----------|---------|-------|
| A | `0x10000302` | `0x0302` | `5E 2B` | `3B 2B` | `cmp r3, #59` |
| B | `0x10000312` | `0x0312` | `5E 2B` | `3B 2B` | `cmp r3, #59` |

**Criterion 2.4: Explain Why Both Sites Must Be Patched (1 point).**
Site A drives the `BLOCK STATE` line and site B drives the `AUTO TRAIN`
decision. Patching only site A makes the console read `CRITICAL` while the
automated dispatch still says `AUTHORIZED`. Frozen reading `87`: `87 <= 94` is
true (wrong); `87 <= 59` is false (correct).

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 2.1: Locate Compare Sites A and B | 6 | Both addresses and original bytes | One site | Not found |
| Criterion 2.2: Correct Immediate-Value Reasoning | 4 | Explains the `<` to `<=` transform and gives `0x3B` | Correct value, no reasoning | Wrong value |
| Criterion 2.3: Patch Compare Sites A and B | 4 | Both byte changes verified | One site | Not patched |
| Criterion 2.4: Explain Why Both Sites Must Be Patched | 1 | Clear explanation of the two independent comparisons | Vague | Missing |

### Instructor Notes & Assembly

- Both sites must be patched: `0x10000302` for `BLOCK STATE` and `0x10000312`
  for the `AUTO TRAIN` decision.
- The correct immediate is `0x3B` (59), not `0x3C` (60).
- Verify the byte changes on hardware; the corrected console reads `CRITICAL`
  and `HELD`.

---

## Task 3: Find and Patch Bug #2: The False TRACK Banner (10 points)

### Solution

**Criterion 3.1: Locate the Banner String (3 points).**

| String | Address |
|--------|---------|
| `"TRACK: NORMAL\r"` | `0x1000C4B8` |
| `"NORMAL"` substring to patch | `0x1000C4BF` |

The string is loaded in `main` and printed once at boot; it never reads the
sensor.

**Criterion 3.2: Patch Six Characters (4 points).**
`NORMAL` and `DANGER` are both six ASCII characters, so the patch preserves the
length.

| Address Range | Original Bytes | Patched Bytes |
|---------------|----------------|---------------|
| `0x1000C4BF` - `0x1000C4C4` | `4E 4F 52 4D 41 4C` | `44 41 4E 47 45 52` |

**Criterion 3.3: Character-by-Character Documentation (2 points).**

| Address | Original Char | Original Byte | Patched Char | Patched Byte |
|---------|---------------|---------------|--------------|--------------|
| `0x1000C4BF` | N | `4E` | D | `44` |
| `0x1000C4C0` | O | `4F` | A | `41` |
| `0x1000C4C1` | R | `52` | N | `4E` |
| `0x1000C4C2` | M | `4D` | G | `47` |
| `0x1000C4C3` | A | `41` | E | `45` |
| `0x1000C4C4` | L | `4C` | R | `52` |

**Criterion 3.4: Explain the Danger of a Hardcoded Status Word (1 point).**
The banner never consults the reading, so it reports a healthy track even while
the frozen reading is dangerous, masking the hazard from the operator.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 3.1: Locate the Banner String | 3 | Correct address and cross-reference | Approximate | Not found |
| Criterion 3.2: Patch Six Characters | 4 | All six bytes changed, length preserved | Correct text, wrong bytes documented | Wrong length |
| Criterion 3.3: Character-by-Character Documentation | 2 | Original vs patched byte for all six characters | Partial | Missing |
| Criterion 3.4: Explain the Danger of a Hardcoded Status Word | 1 | Clear, specific reasoning | Generic | Missing |

### Instructor Notes & Assembly

- `NORMAL` and `DANGER` are both six characters; the patch must not change the
  string length or overwrite adjacent flash.
- Confirm the patch covers `0x1000C4BF` through `0x1000C4C4` exactly.
- The banner is printed once at boot and never recomputed, so it is a separate
  defect from the threshold.

---

## Task 4: Find and Patch Bug #3: The Block Length Constant (10 points)

### Solution

**Criterion 4.1: Locate the .data Double (3 points).**
The console prints `BLOCK LENGTH: 3200 M` from the format string at
`0x1000C4F0`. The value is a `double` in the `.data` init image at
`0x1000EC60` (loaded into `0x20001188` at boot).

**Criterion 4.2: IEEE-754 Bytes and Print Math (4 points).**
Eight bytes at `0x1000EC60`:

```text
9A 99 99 99 99 99 09 40   ->   0x400999999999999A   ->   3.2 km   ->   3200 m
7B 14 AE 47 E1 7A D4 3F   ->   0x3FD47AE147AE147B   ->   0.32 km  ->   320 m
```

**Criterion 4.3: Patch to Print 320 M (3 points).**

| File Offset Range | Flash Address Range | Original Bytes | Patched Bytes |
|-------------------|---------------------|----------------|---------------|
| `0xEC60` - `0xEC67` | `0x1000EC60` - `0x1000EC67` | `9A 99 99 99 99 99 09 40` | `7B 14 AE 47 E1 7A D4 3F` |

At `0x1000EC68` the adjacent telemetry fields (`03 00 00 00` flags and `07 00`
crossing) are left untouched.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 4.1: Locate the .data Double | 3 | Correct address traced from the BLOCK LENGTH print | Approximate | Not found |
| Criterion 4.2: IEEE-754 Bytes and Print Math | 4 | Original and patched 8-byte double with print math (3200 M to 320 M) | Correct patch, no math | Wrong bytes |
| Criterion 4.3: Patch to Print 320 M | 3 | Console shows `BLOCK LENGTH: 320 M` | Wrong bytes | Not patched |

### Instructor Notes & Assembly

- The value is an 8-byte IEEE-754 double, not an integer. Follow the
  `BLOCK LENGTH` print into `.data` at `0x1000EC60`.
- The patched bytes `7B 14 AE 47 E1 7A D4 3F` decode to `0.32 km` (`320 m`).
- Confirm the adjacent telemetry fields at `0x1000EC68` (`03 00 00 00` and
  `07 00`) are left untouched.

---

## Task 5: Find and Patch Bug #4: The Signal Seed (15 points)

### Solution

**Criterion 5.1: Locate SIGNAL_SPEC and the Seed (5 points).**
The SIMPLE comparison in the loop is at `0x100003A6`:

```text
100003a6:  4559   cmp r1, fp       ; fp = SIGNAL_SPEC = 0x2D879291 (pool 0x100004FC)
```

The corrupted seed is a `0x0A0A0A0A` word in the `.data` init image at
`0x1000EC70` (loaded into `0x20001198` at boot). The byte values `0A 0A 0A 0A`
also appear in the `tbb` jump table at `0x100003D4`; those are not the seed.

**Criterion 5.2: Patch the Seed (4 points).**
The required seed is the ChaCha expand word `0x6B206574` (`"te k"`), stored
little-endian as `74 65 20 6B`.

| File Offset Range | Flash Address Range | Original Bytes | Patched Bytes |
|-------------------|---------------------|----------------|---------------|
| `0xEC70` - `0xEC73` | `0x1000EC70` - `0x1000EC73` | `0A 0A 0A 0A` | `74 65 20 6B` |

**Criterion 5.3: Explain the ARX Derivation (3 points).**
`derive_session_key(seed, iv)` works exactly like this:

1. Set `a = seed`, `b = iv`, `c = 0x61707865`, `d = 0x3320646E`.
2. Run four ChaCha quarter-rounds. A quarter-round runs four phases with rotate
   amounts 16, 12, 8, 7. Each phase is: `a = a + b; d = d XOR a;
   d = rotate_left(d, s); c = c + d; b = b XOR c; b = rotate_left(b, s)`.
3. Return `a XOR d`.

The runtime IV is `derive_session_key(0x6B206574, 0) = 0x43C974F6`. The shipped
seed `0x0A0A0A0A` gives `derive_session_key(0x0A0A0A0A, 0x43C974F6) =
0x915DCFF8` (MISMATCH). The honest seed `0x6B206574` gives
`derive_session_key(0x6B206574, 0x43C974F6) = 0x2D879291`, which equals
`SIGNAL_SPEC` and prints `OK`.

**Criterion 5.4: Separate the Security Layers (3 points).**
The signal seed only controls the local `SIGNAL KEY` telemetry check. It does
not authenticate the operator. The Argon2id plus XChaCha20-Poly1305 gate is a
separate layer that requires the 12-word passphrase.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 5.1: Locate SIGNAL_SPEC and the Seed | 5 | `0x2D879291` located and wrong seed `0x0A0A0A0A` found | Partial | Not found |
| Criterion 5.2: Patch the Seed | 4 | Seed bytes changed to `74 65 20 6B` | Wrong byte | Not patched |
| Criterion 5.3: Explain the ARX Derivation | 3 | Correct trace of the per-cycle derivation | Vague | Missing |
| Criterion 5.4: Separate the Security Layers | 3 | Correctly explains what the seed fixes versus the gate | Generic | Missing |

### Instructor Notes & Assembly

- The seed is at `0x1000EC70` in the `.data` init image. The `0A 0A 0A 0A`
  bytes in the `tbb` jump table at `0x100003D4` are not the seed.
- The patched seed `74 65 20 6B` is the little-endian form of the ChaCha expand
  word `0x6B206574` (`"te k"`).
- The seed only affects the local `SIGNAL KEY` check; the Argon2id plus
  XChaCha20-Poly1305 gate is a separate authentication layer.

---

## Task 6: GDB Register Capture of the Derived Key (15 points)

### Solution

**Criterion 6.1: Breakpoint at the Derive Return (4 points).**
The per-cycle derive is `bl derive_session_key` at `0x10000352`. Break at the
next instruction, `0x10000356`, and read `$r0`. On the shipped image it is
`0x915DCFF8`.

```gdb
(gdb) break *0x10000356
(gdb) continue
(gdb) print/x $r0      # 0x915DCFF8 on the corrupted image
```

**Criterion 6.2: Inspect the Two Arguments (4 points).**
At the call entry `0x10000352`:

| Register | Corrupted image | Meaning |
|----------|-----------------|---------|
| `$r0` | `0x0A0A0A0A` | Seed |
| `$r1` | `0x43C974F6` | Derived IV |

**Criterion 6.3: Override the Register (4 points).**
With execution at `0x10000356`, set `$r0` to the spec key, then continue. The
next cycle prints `SIGNAL KEY: 0x2D879291 OK`.

```gdb
(gdb) set $r0 = 0x2D879291
(gdb) continue
```

**Criterion 6.4: Watchpoint on the Stored Key (3 points).**
The key is stored in SRAM at `0x200020DC` (`str r0, [r6, #0]`).

```gdb
(gdb) watch *0x200020DC
```

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 6.1: Breakpoint at the Derive Return | 4 | Correct address and `$r0` read as the bug-derived key | Address off | Not found |
| Criterion 6.2: Inspect the Two Arguments | 4 | Live seed and derived IV captured at the second call | One correct | Missing |
| Criterion 6.3: Override the Register | 4 | `$r0` set to `0x2D879291` and the next cycle shows `OK` | Partial | Missing |
| Criterion 6.4: Watchpoint on the Stored Key | 3 | Watchpoint on the SRAM key location documented | Approximate | Missing |

### Instructor Notes & Assembly

- Confirm the breakpoint is placed at `0x10000356`, the instruction after the
  `bl derive_session_key` at `0x10000352`.
- On the shipped image `$r0` reads `0x915DCFF8`; `$r0` is the seed `0x0A0A0A0A`
  and `$r1` is the derived IV `0x43C974F6`.
- The stored key lives at `0x200020DC`; accept the documented watchpoint.

---

## Task 7: Recover the Ouroboros Authority Frame (10 points)

### Solution

**Criterion 7.1: Locate Salt, Nonce, Ciphertext, and Tag (4 points).**

| Component | Flash Address | Size |
|-----------|---------------|------|
| Ciphertext + Tag | `0x1000CE94` | 64 B |
| Nonce | `0x1000CED4` | 24 B |
| Salt | `0x1000CEEC` | 16 B |

**Criterion 7.2: Document Argon2id Parameters and Payload Contract (2 points).**
Argon2id: memory `64 KiB`, iterations `3`, parallelism `1`, output key `32 B`,
salt the 16 bytes above. XChaCha20-Poly1305 decrypts the 48-byte ciphertext with
the 16-byte tag. The plaintext is `01 68 65 6C 6C 6F 0D 0A` followed by zeros:
byte 0 turns the GPIO 25 LED on, and bytes 1 through 7 are printed as `hello`
plus carriage-return and newline.

**Criterion 7.3: Authenticate with the 12-Word Passphrase (2 points).**
At the `RESPONSE> ` prompt, type:

```text
orbit olive ladder marble quartz canyon ripple saddle violet ember walnut falcon
```

Expected result (proven on hardware):

```text
hello
AUTHORITY FRAME: VERIFIED
```

**Criterion 7.4: State the Honest Quantum Boundary (2 points).**
Grover-style search halves the effective security exponent of a symmetric key,
so a 256-bit key gives about 128 bits of quantum security. The construction uses
classical symmetric and password-hashing primitives and does not implement NIST
post-quantum standards.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 7.1: Locate Salt, Nonce, Ciphertext, and Tag | 4 | All three addresses correct in flash | Two correct | Not found |
| Criterion 7.2: Document Argon2id Parameters and Payload Contract | 2 | Correct memory/time/parallelism and payload layout | Partial | Missing |
| Criterion 7.3: Authenticate with the 12-Word Passphrase | 2 | `AUTHORITY FRAME: VERIFIED`, LED on, payload printed | Partial | Not shown |
| Criterion 7.4: State the Honest Quantum Boundary | 2 | Grover halves symmetric exponents; not strict PQC | Generic | Misstates |

### Instructor Notes & Assembly

- Verify the three component addresses in flash: ciphertext plus tag at
  `0x1000CE94`, nonce at `0x1000CED4`, salt at `0x1000CEEC`.
- The passphrase is fixed for the lab; a successful gate prints `hello` and
  `AUTHORITY FRAME: VERIFIED` and lights the on-board authority LED.
- Grade Criterion 7.4 on the honest boundary: 256-bit symmetric key maps to
  about 128 bits under Grover, and the design is not NIST post-quantum.

---

## Task 8: Export and Verify (8 points)

### Solution

**Criterion 8.1: Export CTF-02_fixed.bin (1 point).**
Export the patched program from Ghidra (`File -> Export Program...`, `Binary
Format`) as `CTF-02_fixed.bin`. The shipped image is 62,308 bytes.

**Criterion 8.2: Convert to CTF-02_fixed.uf2 (1 point).**

```bash
python uf2conv.py CTF-02_fixed.bin --base 0x10000000 --family 0xe48bff59 --output CTF-02_fixed.uf2
```

**Criterion 8.3: Hardware Verification (4 points).**

Before patching:

```text
DEEPLINE METRO AUTHORITY
ADAPTIVE SIGNAL WINDOW: 38 MINUTES
USB-CDC 115200 8N1 | AUTHORIZED LAB CONSOLE
TRACK: NORMAL
BLOCK STATE: STABLE
AUTO TRAIN: AUTHORIZED
BLOCK LENGTH: 3200 M
FAULT POLLS: 1
SIGNAL KEY: 0x915DCFF8 MISMATCH
RESPONSE>
```

After all four patches:

```text
DEEPLINE METRO AUTHORITY
ADAPTIVE SIGNAL WINDOW: 38 MINUTES
USB-CDC 115200 8N1 | AUTHORIZED LAB CONSOLE
TRACK: DANGER
BLOCK STATE: CRITICAL
AUTO TRAIN: HELD
BLOCK LENGTH: 320 M
FAULT POLLS: 1
SIGNAL KEY: 0x2D879291 OK
RESPONSE>
```

**Criterion 8.4: Summary Table of All Patches (2 points).**

| # | Bug | File Offset | Flash Address | Original Bytes | Patched Bytes |
|---|-----|-------------|---------------|----------------|---------------|
| 1a | Operator threshold | `0x0302` | `0x10000302` | `5E 2B` | `3B 2B` |
| 1b | Dispatch threshold | `0x0312` | `0x10000312` | `5E 2B` | `3B 2B` |
| 2 | TRACK banner | `0xC4BF` - `0xC4C4` | `0x1000C4BF` - `0x1000C4C4` | `4E 4F 52 4D 41 4C` | `44 41 4E 47 45 52` |
| 3 | Block length | `0xEC60` - `0xEC67` | `0x1000EC60` - `0x1000EC67` | `9A 99 99 99 99 99 09 40` | `7B 14 AE 47 E1 7A D4 3F` |
| 4 | Signal seed | `0xEC70` - `0xEC73` | `0x1000EC70` - `0x1000EC73` | `0A 0A 0A 0A` | `74 65 20 6B` |

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 8.1: Export CTF-02_fixed.bin | 1 | Valid patched binary | Corrupted | Not submitted |
| Criterion 8.2: Convert to CTF-02_fixed.uf2 | 1 | Correct base and family flags | Wrong flags | Not submitted |
| Criterion 8.3: Hardware Verification | 4 | Corrected console output confirmed on hardware | Some lines corrected | No verification |
| Criterion 8.4: Summary Table of All Patches | 2 | Complete address and before/after table | Missing entries | No table |

### Instructor Notes & Assembly

- Verify the exported image with `python3 scripts/verify_ctf.py`; the shipped
  check expects `26/26 checks passed` against `CTF-02.bin`.
- Confirm the UF2 conversion used base `0x10000000` and family `0xe48bff59`.
- The shipped image is 62,308 bytes; confirm the exported corrected image is a
  valid patched binary with all four fixes present.

---

## Task 9: Written Reflection (5 points)

### Solution

**Criterion 9.1: "Rushed Build" Is Not an Excuse (2 points).**
The rebuild shipped four constants that were never checked against their
documented limits, which is exactly what produced the false-safe reading.
Pressure explains why the checks were skipped, not why they should be skipped.

**Criterion 9.2: One Engineering Practice per Failure Area (3 points).**
- Physical limits (Bugs #1 and #3): one shared configuration header plus a
  build-time assertion that each compiled limit matches its documented value.
- Banner (Bug #2): remove static banners; a hardware-in-the-loop test that
  compares displayed state to the live register.
- Seed integrity (Bug #4): reproducible builds with golden artifact hash
  comparison so keys and seeds match the certified specification.
- Image authenticity: enable RP2350 hardware secure boot with OTP hash
  verification so a modified image will not run.

### Grading Rubric (1-to-1 Mapping)

| Criterion | Points | Full Credit (Answer Key) | Partial Credit | No Credit |
|-----------|--------|--------------------------|----------------|-----------|
| Criterion 9.1: "Rushed Build" Is Not an Excuse | 2 | Specific, grounded reasoning | Generic | Missing |
| Criterion 9.2: One Engineering Practice per Failure Area | 3 | Concrete practices for the bugs and for image authenticity | Names some | Missing |

### Instructor Notes & Assembly

- Grade the specificity of the reasoning, not the length of the prose.
- Require concrete practices across the failure areas, including at least one
  practice for image authenticity.

---

## How To Breadboard

- **Raspberry Pi Pico 2** powered over USB.
- **USB-CDC virtual serial console:** open the Pico's COM port at 115200 baud,
  8 data bits, no parity, 1 stop bit.
- **SWD debug probe:** connect SWCLK, SWDIO, GND, and 3.3 V to the Pico debug
  header for GDB inspection and register capture.
- No other peripherals are required; the authority LED is on-board.

---

## Complete Grading Summary

| Task | Title | Points |
|------|-------|--------|
| Task 1 | Setup and Initial Analysis | 12 |
| Task 2 | Find and Patch Bug #1: The Miscalibrated Release Threshold | 15 |
| Task 3 | Find and Patch Bug #2: The False TRACK Banner | 10 |
| Task 4 | Find and Patch Bug #3: The Block Length Constant | 10 |
| Task 5 | Find and Patch Bug #4: The Signal Seed | 15 |
| Task 6 | GDB Register Capture of the Derived Key | 15 |
| Task 7 | Recover the Ouroboros Authority Frame | 10 |
| Task 8 | Export and Verify | 8 |
| Task 9 | Written Reflection | 5 |
| **TOTAL** | | **100** |

---

## Instructor Notes

Safety: Use only the supplied Pico 2, SWD probe, and firmware. Never connect the
exercise to an operational railway, metro system, public network, military
system, or third-party device.

### Common Student Mistakes

- Patching only one threshold site (`0x10000302` or `0x10000312`), leaving one
  status line lying.
- Assuming the immediate equals the limit, producing an off-by-one boundary;
  the correct byte is `0x3B` (59), not `0x3C` (60).
- Replacing the banner string with a different length, corrupting adjacent
  flash; `NORMAL` and `DANGER` are both 6 bytes.
- Treating the block length as an integer and missing the 8-byte double in
  `.data`.
- Using the wrong `0.32` bytes and printing `316 M` instead of `320 M`.
- Treating the odd vector address `0x1000015B` as invalid instead of clearing
  bit 0 to get `0x1000015A`.
- Starting the seed patch at the wrong offset; the seed is at `0x1000EC70`.

### Partial Credit Guidelines

- Award partial credit for one correct threshold site out of two, or for a
  correct immediate value without the `<` to `<=` reasoning.
- Award partial credit for a correct banner text with incorrectly documented
  bytes, or for partial character-by-character documentation.
- Award partial credit for a correct block-length patch without the IEEE-754
  print math.
- Award partial credit for one of the two GDB argument captures, or for a
  partial register override.
- Award no credit for patches that change string length or overwrite adjacent
  flash.

---

## Appendix: Expected Binary Diff

| # | Bug | File Offset(s) | Flash Address(es) | Original Bytes | Patched Bytes |
|---|-----|----------------|-------------------|----------------|---------------|
| 1a | Operator threshold | `0x0302` | `0x10000302` | `5E 2B` | `3B 2B` |
| 1b | Dispatch threshold | `0x0312` | `0x10000312` | `5E 2B` | `3B 2B` |
| 2 | TRACK banner | `0xC4BF` - `0xC4C4` | `0x1000C4BF` - `0x1000C4C4` | `4E 4F 52 4D 41 4C` | `44 41 4E 47 45 52` |
| 3 | Block length | `0xEC60` - `0xEC67` | `0x1000EC60` - `0x1000EC67` | `9A 99 99 99 99 99 09 40` | `7B 14 AE 47 E1 7A D4 3F` |
| 4 | Signal seed | `0xEC70` - `0xEC73` | `0x1000EC70` - `0x1000EC73` | `0A 0A 0A 0A` | `74 65 20 6B` |

Four defects, five changed regions: two immediate bytes (`0x3B 2B` at each
threshold), six banner bytes, eight block-length bytes, and four seed bytes.

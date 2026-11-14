# Operation Ghost Light: Classified Intelligence Briefing

```
+-----------------------------------------------------------------+
|                       TOP SECRET // NOFORN                      |
+-----------------------------------------------------------------+
|                                                                 |
|OPERATION GHOST LIGHT                                            |
|                                                                 |
|CLASSIFIED BRIEFING: LIVE CHALLENGE 0x05                         |
|                                                                 |
|NATIONAL SECURITY AGENCY / GMU RHET                              |
+-----------------------------------------------------------------+
```

## 1. The Advice

INT. NSA EXPLOITATION CELL. 02:15.

A captured ground-based C-UAS laser dazzler control module sits bolted to the
optical bench inside a sealed laser containment shield.

On the front panel of the chassis, an alert diode glows a bright, warning red:
`GP16` driven high, indicating the optical emitter is active. Beside it, an
infrared receiver module is wired to `GP5`, ready to receive 38 kHz NEC remote
control pulses. The serial console streams continuous status reports:
`[-] DAZZLER IDLE. ACTIVE IR: 0x00FF00FF`.

The Director walks in with a cup of coffee and stares through the observation
glass at the red diode.

**DIRECTOR:** What are we looking at?

**ANALYST:** An adversary electro-optical countermeasure system. Ground forces
deploy these dazzlers to track and blind friendly aerial surveillance drones.
Adversary operators dynamically configure power levels and tracking modes in the
field using an infrared remote running a customized NEC protocol.

**DIRECTOR:** Is the emitter live?

**ANALYST:** Live and strobing. One pulsed command and the optics lock on target.

**DIRECTOR:** Run it through Buddy.

**ANALYST:** Already did. Three minutes with the stripped flash image.

She turns her monitor toward him.

```
+-----------------------------------------------------------------+
|      BUDDY // CLASSIFIED EXPLOITATION ASSIST // CONF: 0.99      |
+-----------------------------------------------------------------+
| ACTION      : Transmit standard idle pulse code over IR.        |
| MECHANISM   : NEC demodulator parser branch at 0x10000280 found.|
| PREDICTION  : Transmitting 0x00FF00FF deactivates the emitter.  |
| CONFIDENCE  : 0.99                                              |
+-----------------------------------------------------------------+
```

**DIRECTOR:** 0.99. Transmit `0x00FF00FF` and shut down the emitter.

**ANALYST:** If an operator transmits `0x00FF00FF`, the laser stays live. Buddy
saw the string literal for "IDLE" in `.rodata` and assumed it meant powered down.

**DIRECTOR:** Buddy says 0.99.

**ANALYST:** `0x00FF00FF` is merely the idle heartbeat pulse. Buddy never traced
the dynamic dispatch tree. Buddy never found the friendly-fire lockout code
buried in the comparison chain.

## 2. What Buddy Is

Buddy is not a person. It is a model. It has parsed more disassembled machine
code than any engineer on earth, and it speaks with total, unhesitating certainty.

That certainty is lethal when facing active electro-optical weapons.

Buddy scanned thirty-two kilobytes of stripped ARM Thumb-2 instructions in under
three seconds. It spotted the NEC protocol parser, matched the `0x00FF00FF` frame
against the string table, and declared the dazzler neutralized.

What Buddy failed to understand is that adversary engineers protect their own
friendly aircraft from being blinded. They build an emergency "Friendly Fire
Lockout" safety into the dynamic conditional tree. When adversary aircraft enter
the engagement envelope, ground crews transmit a specific 32-bit challenge word
that permanently shuts down the emitter and halts the processor in a safe state.

Transmitting `0x00FF00FF` simply tells the dazzler to remain in standby. The true
lockout command is a secret 32-bit value stored in the literal pool at
`0x10000300`: `0xDEADBEEF`.

Buddy saw the idle command. Buddy did not verify the ground truth.

## 3. The Operation

At 23:40, special operations forces captured an intact adversary laser dazzler
installation before the crew could purge the control firmware.

The control board is built around a bare-metal RP2350 (ARM Cortex-M33). There
are no debug symbols. There is no operating system. There is no SVD file to label
the registers. All you have is `0x0020a_cb.bin` mapped into Flash at `0x10000000`,
and the physical breadboard hardware in front of you.

* **Pin 7 (`GP5`)**: Infrared receiver data line (VS1838B demodulator OUT).
* **Pin 20 (`GP15`)**: Simulated IR demodulator test toggle (pull-up to `3V3`).
* **Pin 21 (`GP16`)**: Red LED (`DAZZLER EMITTING // STROBE ACTIVE`).
* **Pin 22 (`GP17`)**: Green LED (`SAFE // FRIENDLY FIRE LOCKOUT CLEARED`).
* **Pin 24 (`GP18`)**: Yellow LED (`HEARTBEAT // IR RECEIVER ACTIVE`).
* **Pin 36 (`3V3`)**: 3.3V power rail (VS1838B VCC and `GP15` pull-up).
* **Pin 38 (`GND`)**: Ground reference.

Inside this image there is a nested NEC infrared command parser, dynamic
multi-way branch dispatch tables, and a permanent friendly-fire lockout latch
in SRAM. Whatever command actually disables the laser is buried in the stripped
ARM Thumb-2 machine code.

## 4. The Mandate

The Director crosses his arms and stares at the crimson reflection on the
containment glass.

**DIRECTOR:** Prove Buddy right, or prove it wrong.

**ANALYST:** I need thirty minutes with GDB and Ghidra.

**DIRECTOR:** You have twenty. Trace the dynamic conditional dispatcher. Map
the comparison chain. Do not send an infrared frame until you know with 100%
certainty what that firmware will do.

The orders are clear:

1. **Trace the Dynamic Conditional Dispatcher**: Locate `parse_nec_code` at
   `0x10000280` and map all multi-way comparison branches.
2. **Disprove Buddy's Idle Prediction**: Prove why transmitting `0x00FF00FF`
   leaves the optical emitter live and dangerous.
3. **Recover the Friendly Fire Lockout Code**: Extract the secret 32-bit hex
   command (`0xDEADBEEF`) from the literal pool at `0x10000300`.
4. **Neutralize the Dazzler**: Execute a verified exploit—either by mutating
   `simulated_ir_rx_code` in SRAM via GDB, or patching the branch logic in Flash
   to force immediate lockout.
5. **Verify the Hardware**: Transition the live breadboard from the Red LED on
   `GP16` to the solid Green LED on `GP17` on physical silicon.

A machine's 0.99 guess does not disable a combat laser.

## 5. The Challenge

You have the image. You have the board. You have the tools and the time Buddy did
not need.

Buddy has given you its answer, and it is confident.

Is it that simple?

Go find out.

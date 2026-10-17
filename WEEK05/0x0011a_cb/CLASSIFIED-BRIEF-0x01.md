# Operation Dark Vector: Classified Intelligence Briefing

```
+-----------------------------------------------------------------+
|                       TOP SECRET // NOFORN                      |
+-----------------------------------------------------------------+
|                                                                 |
|OPERATION DARK VECTOR                                            |
|                                                                 |
|CLASSIFIED BRIEFING: LIVE CHALLENGE 0x01                         |
|                                                                 |
|NATIONAL SECURITY AGENCY / GMU RHET                              |
+-----------------------------------------------------------------+
```

## 1. The Advice

INT. NSA EXPLOITATION CELL. 05:03.

A carbon-fiber forward fuselage section of a downed state-sponsored autonomous
micro-UAV sits clamped in an anti-static fixture on the bench.

On the front of the avionics bay, a status diode glows a steady red: `GP16` high.
Beside it, an I2C 1602 LCD backpack is wired to `GP2` and `GP3`, outputting
telemetry coordinates. A mock propeller servo is wired to `GP6`, a u-blox NEO-6M
GPS receiver is wired to PIO UART RX on `GP7`, and a REYAX RYLR998 LoRa transceiver
is wired to `UART1` on `GP8` and `GP9`. A thin jumper loops from `GP15` up to the
`3V3` rail.

The Director sets down a field report and looks at the red diode.

**DIRECTOR:** Where was it heading?

**ANALYST:** Buddy says it knows.

**DIRECTOR:** Buddy.

**ANALYST:** The model. The one we built for automated firmware triage. Give it
the image, ask it the question.

**DIRECTOR:** And?

**ANALYST:** Nine seconds with the stripped flash image.

She turns her monitor toward him.

```
+-----------------------------------------------------------------+
|      BUDDY // CLASSIFIED EXPLOITATION ASSIST // CONF: 0.98      |
+-----------------------------------------------------------------+
| ACTION      : Recover target waypoint coordinates.              |
| MECHANISM   : Literal pool constants in .rodata parsed.         |
| PREDICTION  : Destination is +38.840280, -77.428890.            |
| CONFIDENCE  : 0.98                                              |
+-----------------------------------------------------------------+
```

**DIRECTOR:** 0.98. Centreville, Virginia. That is the whole answer.

**ANALYST:** Buddy found the pre-programmed decoy coordinate in `.rodata`. Buddy
never looked at SRAM. Buddy never audited the cryptographic routine decrypting
the true mission target at boot.

**DIRECTOR:** Buddy says 0.98.

**ANALYST:** Buddy saw a float pair in Flash and stopped looking. If that drone
were flying to Centreville, it would not be broadcasting those exact coordinates
in plaintext telemetry for every SIGINT sensor in the sector to hear.

## 2. What Buddy Is

Buddy is not a person. It is a model. It has parsed more disassembled machine
code than any engineer on earth, and it speaks with total, unhesitating certainty.

That certainty is lethal when tracking an adversary strike platform.

Buddy scanned the flash image, spotted two IEEE-754 double-precision floating-point
constants matching geographic coordinates, and concluded that finding the floats
is the whole game. It extracted the numbers, declared the target solved, and moved
on.

What Buddy failed to consider is that adversary weapons engineers anticipate
capture and forensic recovery. They do not hardcode high-value strategic targets
in plaintext Flash memory. A real drone stores decoy launch coordinates in Flash,
while encrypting its true destination under 128-bit AES. At power-on, firmware
decrypts the authentic waypoint directly into SRAM before engaging guidance.

The plaintext coordinates in Flash are bait. Buddy took the bait.

Buddy saw the floating-point constants. Buddy did not verify the ground truth.

## 3. The Operation

At 04:17, radar pickets tracked an adversary one-way micro-UAV descending into
the woods outside Centreville, Virginia after battery exhaustion. Technical
Intelligence operators recovered the airframe before adversary recovery forces
could zeroize the avionics.

The flight computer is built around a bare-metal RP2350 (ARM Cortex-M33). There
are no debug symbols. There is no operating system. There is no SVD file to label
the registers. All you have is `0x0011a_cb.bin` mapped into Flash at `0x10000000`,
and the physical breadboard hardware in front of you.

* **Pin 4 (`GP2`)**: I2C1 SDA (to 1602 LCD backpack).
* **Pin 5 (`GP3`)**: I2C1 SCL (to 1602 LCD backpack).
* **Pin 9 (`GP6`)**: SG90 servo PWM mock propeller blade.
* **Pin 10 (`GP7`)**: PIO UART RX (from u-blox NEO-6M GPS TXD).
* **Pin 11 (`GP8`)**: LoRa UART1 TX (to RYLR998 RXD).
* **Pin 12 (`GP9`)**: LoRa UART1 RX (from RYLR998 TXD).
* **Pin 20 (`GP15`)**: Mission telemetry sync trigger (pull-up to `3V3`).
* **Pin 21 (`GP16`)**: Red LED (`NO GNSS FIX // UNGUIDED SEARCH`).
* **Pin 22 (`GP17`)**: Green LED (`TARGET REACHED // PAYLOAD RELEASED`).
* **Pin 24 (`GP18`)**: Yellow LED (`SATELLITES ACQUIRED // 3D FIX LOCK`).
* **Pin 36 (`3V3`)**: 3.3V power rail (RYLR998 VDD and GPS VCC).
* **Pin 38 (`GND`)**: Ground reference.
* **Pin 40 (`VBUS`)**: 5V power rail (SG90 servo VCC and LCD power).

Inside this image there is an AES-128 ECB decryption routine, an encrypted 16-byte
target coordinate block in `.rodata`, double-precision floating-point navigation
math, and dynamic GNSS state evaluation. Whatever coordinates actually steer the
aircraft are buried in the stripped ARM Thumb-2 machine code.

## 4. The Mandate

The Director crosses his arms.

**DIRECTOR:** Prove Buddy right, or prove it wrong.

**ANALYST:** I need thirty minutes with GDB and Ghidra.

**DIRECTOR:** You have twenty. Reverse the boot sequence. Audit the memory
structures. Do not sign off on that flight path until you have extracted the true
target from silicon and verified it bit-for-bit.

The orders are clear:

1. **Disprove the Decoy Coordinates**: Locate the pre-programmed coordinates in
   Flash and prove why they represent a decoy origin rather than the real target.
2. **Recover the AES-128 Key and Ciphertext**: Identify the 16-byte ciphertext
   block and the 16-byte AES key embedded within `.rodata`.
3. **Decrypt the True Destination**: Reverse the boot decryption sequence and
   compute the true target latitude and longitude in double-precision IEEE-754.
4. **Neutralize the Flight Plan**: Execute a verified exploit—either by mutating
   the destination coordinates in SRAM via GDB or patching the binary in Flash.
5. **Verify the Hardware**: Transition the live breadboard from the Red LED on
   `GP16` to the solid Green LED on `GP17` on physical silicon.

A machine's 0.98 guess does not authorize an airstrike.

## 5. The Challenge

You have the image. You have the board. You have the tools and the time Buddy did
not need.

Buddy has given you its answer, and it is confident.

Is it that simple?

Go find out.

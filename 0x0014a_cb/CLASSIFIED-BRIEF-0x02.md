# Operation Zero Hour: Classified Intelligence Briefing

```
+-----------------------------------------------------------------+
|                       TOP SECRET // NOFORN                      |
+-----------------------------------------------------------------+
|                                                                 |
|OPERATION ZERO HOUR                                              |
|                                                                 |
|CLASSIFIED BRIEFING: LIVE CHALLENGE 0x02                         |
|                                                                 |
|NATIONAL SECURITY AGENCY / GMU RHET                              |
+-----------------------------------------------------------------+
```

## 1. The Advice

INT. NSA EXPLOITATION CELL. 05:45.

The air in the secure chamber is cold and smells of machine oil. Secured in an
aluminum shock-mount cradle on the bench sits the recovered forward guidance and
fuze assembly of an adversary loitering munition.

On the front plate of the Electronic Safe-and-Arm (ESA) housing, an unblinking
red diode burns: `GP16` high. Beside it, the antenna of a REYAX RYLR998 LoRa
transceiver extends from the chassis, connected to `UART1` on `GP8` and `GP9`.

The Director walks in, carrying a lukewarm mug of black coffee. He stares at the
red diode.

**DIRECTOR:** Is the fuze live?

**ANALYST:** Live and armed. The drone suffered an engine flameout and pancaked
into soft marshland. The impact deceleration was insufficient to crush the piezo
striker, but the warhead's electronic arming logic is fully engaged.

**DIRECTOR:** How volatile is it?

**ANALYST:** One mistake on the radio link and the secondary explosive train fires.

**DIRECTOR:** Did you run the binary through Buddy?

**ANALYST:** I fed Buddy the stripped flash dump four minutes ago.

She pivots her high-resolution workstation monitor toward him.

```
+-----------------------------------------------------------------+
|      BUDDY // CLASSIFIED EXPLOITATION ASSIST // CONF: 0.99      |
+-----------------------------------------------------------------+
| ACTION      : Transmit generic abort frame over LoRa (915 MHz). |
| MECHANISM   : uart1 packet ingestion on GP8/GP9 detected.       |
| PREDICTION  : RF abort packet clears latch and engages GP17.   |
| CONFIDENCE  : 0.99                                              |
+-----------------------------------------------------------------+
```

**DIRECTOR:** 0.99 confidence. Transmit the abort frame and render it safe.

**ANALYST:** If an operator transmits an unverified packet right now, this building
ceases to exist.

**DIRECTOR:** Buddy says 0.99.

**ANALYST:** Buddy saw the call to `lora_poll_packet`. Buddy never looked at SRAM.
Buddy never looks at the state machine.

## 2. What Buddy Is

Buddy is not a person. It is a model. It has parsed more disassembled machine
code than any engineer on earth, and it speaks with total, unhesitating certainty.

That certainty is lethal when dealing with live ordnance.

Buddy scanned thirty-two kilobytes of raw ARM Thumb-2 instructions in under four
seconds. It spotted the UART1 packet parser on GPIO 8 and 9, observed that it
interacted with the arming logic, and declared victory.

What Buddy failed to consider is that real adversary weapons are built by
engineers who anticipate capture and signal spoofing. They know an allied signals
intelligence unit will attempt to broadcast RF command abort frames.

Inside a real fuze, a radio receiver rarely connects to a simple on/off latch.
Adversary firmware uses state machines, hidden validation logic, and anti-tamper
traps. Transmitting blindly over the air before verifying what happens to that
packet inside silicon is how bomb disposal teams get killed.

Buddy saw the radio receiver. Buddy did not verify the ground truth.

## 3. The Operation

At 03:15, radar pickets tracked an adversary one-way loitering attack drone
descending over contested terrain. Technical Intelligence operators recovered the
airframe before adversary recovery forces could zeroize the avionics.

The brain of the Electronic Safe-and-Arm assembly is a bare-metal RP2350 (ARM
Cortex-M33). There are no debug symbols. There is no operating system. There is no
SVD file to label the registers.

All you have is `0x0014a_cb.bin` mapped into Flash at `0x10000000`, and the
physical breadboard hardware sitting in front of you.

* **Pin 11 (`GP8`)**: LoRa UART1 TX (connected to RYLR998 RXD).
* **Pin 12 (`GP9`)**: LoRa UART1 RX (connected to RYLR998 TXD).
* **Pin 20 (`GP15`)**: Anti-tamper power sense line (connected to `3V3`).
* **Pin 21 (`GP16`)**: Red LED (`WARHEAD ARMED // SENSORS LIVE`).
* **Pin 22 (`GP17`)**: Green LED (`WARHEAD DISARMED // SYSTEM SAFE`).
* **Pin 36 (`3V3`)**: REYAX RYLR998 VDD power rail and GP15 pull-up source.
* **Pin 38 (`GND`)**: Ground reference.

Adversary engineers do not leave their fuzing mechanisms unguarded. Whatever logic,
tokens, or defenses govern this weapon are buried inside thirty-two kilobytes of
stripped ARM Cortex-M33 machine code.

## 4. The Mandate

The Director sets his mug down and looks at the red diode illuminating the bench
in sharp crimson.

**DIRECTOR:** Prove Buddy right, or prove it wrong.

**ANALYST:** I need thirty minutes with GDB and Ghidra.

**DIRECTOR:** You have twenty. Trace the packet handler. Map the state machine.
Do not transmit a single byte over LoRa until you know with 100% certainty what
that firmware will do.

The orders are clear:

1. **Investigate the RF Receiver Logic**: Disassemble the firmware and trace what
   happens when data arrives on UART1 (`GP8`/`GP9`).
2. **Prove or Disprove Buddy's Prediction**: Determine whether transmitting an
   abort frame renders the fuze safe, or whether Buddy missed an internal trap.
3. **Neutralize the Munition**: Execute a verified exploit—either by discovering
   the valid authorization mechanism, mutating the state in SRAM via GDB, or
   patching the binary in Flash.
4. **Verify the Hardware**: Transition the live breadboard from the Red LED on
   `GP16` to the solid Green LED on `GP17` on physical silicon.

A machine's 0.99 guess does not risk a detonation.

## 5. The Challenge

You have the image. You have the board. You have the tools and the time Buddy
did not need.

Buddy has given you its answer, and it is confident.

Is it that simple?

Go find out.

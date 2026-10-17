# Operation Broken Wing: Classified Intelligence Briefing

```
+-----------------------------------------------------------------+
|                       TOP SECRET // NOFORN                      |
+-----------------------------------------------------------------+
|                                                                 |
|OPERATION BROKEN WING                                            |
|                                                                 |
|CLASSIFIED BRIEFING: LIVE CHALLENGE 0x04                         |
|                                                                 |
|NATIONAL SECURITY AGENCY / GMU RHET                              |
+-----------------------------------------------------------------+
```

## 1. The Advice

INT. NSA EXPLOITATION CELL. 03:25.

A heavy-lift composite UAS payload cradle sits clamped to the steel bench, its
servicing hatch removed. Suspended beneath the chassis is a sealed canister
secured by an SG90 servo motor actuator latch.

On the front plate of the payload controller, a status diode glows a sharp red:
`GP16` driven high. Beside it, the USB debug interface streams periodic
telemetry frames to a monitoring terminal:
`[-] ALTITUDE 0. PWM LOCKED: SAFETY ENGAGED.`

The Director enters the secure bay, sets down a mission folder, and stares at
the red diode.

**DIRECTOR:** What is in that bay?

**ANALYST:** Hazardous cargo. An adversary heavy-lift transport drone brought
down intact by a kinetic net. The airframe carries a chemical dispersal bay
secured by an SG90 servo motor. The flight controller uses a static conditional
state machine to ensure the payload only drops when the aircraft is explicitly
armed and above release altitude.

**DIRECTOR:** Is the release mechanism active?

**ANALYST:** Armed and evaluating. One wrong signal on the PWM line or a dropped
logic state and the release latch drops the canister onto the bench.

**DIRECTOR:** Run it through Buddy.

**ANALYST:** Already did. Four minutes with the stripped flash image.

She turns her monitor toward him.

```
+-----------------------------------------------------------------+
|      BUDDY // CLASSIFIED EXPLOITATION ASSIST // CONF: 0.99      |
+-----------------------------------------------------------------+
| ACTION      : Force low-altitude barometric sensor override.    |
| MECHANISM   : Integer comparison in process_payload_state found.|
| PREDICTION  : Setting altitude to 150m triggers payload release.|
| CONFIDENCE  : 0.99                                              |
+-----------------------------------------------------------------+
```

**DIRECTOR:** 0.99. Force 150 meters and release the payload for EOD inspection.

**ANALYST:** If an operator forces 150 meters right now, nothing happens. Worse,
if they fumble the state machine, the safety locks permanently or the cargo bay
latches shut.

**DIRECTOR:** Buddy says 0.99.

**ANALYST:** Buddy saw `cmp.w r0, #500`. Buddy assumed a low altitude reading on
the ground drops the cargo. Buddy completely missed the branch condition: the
firmware uses `bgt.n`—it only releases when altitude is strictly greater than 500
meters! Buddy never examined the static state machine in SRAM.

## 2. What Buddy Is

Buddy is not a person. It is a model. It has parsed more disassembled machine
code than any engineer on earth, and it speaks with total, unhesitating certainty.

That certainty is lethal when handling hazardous cargo fuzing.

Buddy scanned thirty-two kilobytes of stripped ARM Thumb-2 instructions in under
four seconds. It spotted the integer comparison against 500, assumed that cargo
delivery drops occur near ground level, and declared the challenge solved.

What Buddy failed to consider is that real adversary cargo drop systems are
engineered for high-altitude parachute extraction. They do not drop their canisters
at low altitudes where ground turbulence and small-arms fire can destroy the
carrier. The conditional state machine enforces two strict prerequisites:
`current_state` must be `STATE_ARMED`, and `current_altitude` must exceed 500
meters.

On the ground bench at 0 meters, the servo stays locked at zero duty cycle. If
an operator forces 150 meters, the conditional branch fails, keeping safety
latches fully engaged.

Buddy saw the comparison instruction. Buddy did not verify the ground truth.

## 3. The Operation

At 02:40, a kinetic counter-UAS interceptor brought down a hostile heavy-lift
drone over an exclusion zone. EOD recovered the intact avionics and payload
assembly before adversary self-destruct timers could engage.

The flight controller is built around a bare-metal RP2350 (ARM Cortex-M33). There
are no debug symbols. There is no operating system. There is no SVD file to label
the registers. All you have is `0x001da_cb.bin` mapped into Flash at `0x10000000`,
and the physical breadboard hardware in front of you.

* **Pin 20 (`GP15`)**: Altimeter telemetry sync toggle (pull-up to `3V3`).
* **Pin 21 (`GP16`)**: SG90 servo PWM signal / Red LED (`PAYLOAD LOCKED`).
* **Pin 22 (`GP17`)**: Green LED (`PAYLOAD DEPLOYED // BAY OPEN`).
* **Pin 24 (`GP18`)**: Yellow LED (`HEARTBEAT // GUIDANCE ACTIVE`).
* **Pin 36 (`3V3`)**: 3.3V logic reference and pull-up rail.
* **Pin 38 (`GND`)**: Ground reference (common ground for servo and Pico 2).
* **Pin 40 (`VBUS`)**: 5V power rail (SG90 servo motor VCC).

Inside this image there is a static conditional state machine, an RP2350 PWM
slice generator driving the servo actuator, and persistent state variables in
SRAM. Whatever mechanism actually commands the release latch is buried in the
stripped ARM Thumb-2 machine code.

## 4. The Mandate

The Director sets his folder down and looks at the red diode illuminating the
bench.

**DIRECTOR:** Prove Buddy right, or prove it wrong.

**ANALYST:** I need thirty minutes with GDB and Ghidra.

**DIRECTOR:** You have twenty. Trace the comparison branches. Map the state
machine in SRAM. Do not pulse that servo line until you know with 100% certainty
what that firmware will do.

The orders are clear:

1. **Locate the State Machine Handler**: Disassemble the firmware and locate
   `process_payload_state` at `0x100002DC`.
2. **Disprove Buddy's Altitude Prediction**: Analyze the `cmp.w` and branch
   instructions to prove why 150 meters fails to release the cargo latch.
3. **Map the SRAM State Variables**: Inspect `drone_state_t` in SRAM
   (`0x20001180`), isolating `current_state` and `current_altitude`.
4. **Safely Deploy the Payload Bay**: Execute a verified exploit—either by
   mutating the altitude to $> 500$ in SRAM via GDB, or patching the conditional
   branch in Flash (`0x100002EA`) to force immediate release on the bench.
5. **Verify the Hardware**: Observe the SG90 servo motor physically rotate to the
   deployed position (`pwm_set_gpio_level` to 4000) and verify the state
   transition on physical silicon.

A machine's 0.99 guess does not authorize an unverified drop.

## 5. The Challenge

You have the image. You have the board. You have the tools and the time Buddy did
not need.

Buddy has given you its answer, and it is confident.

Is it that simple?

Go find out.

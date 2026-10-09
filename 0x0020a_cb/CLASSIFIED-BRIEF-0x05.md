# Operation Ghost Light: Classified Intelligence Briefing

```
+-----------------------------------------------------------------+
|                       TOP SECRET // NOFORN                      |
+-----------------------------------------------------------------+
|                                                                 |
|OPERATION GHOST LIGHT                                            |
|                                                                 |
|CLASSIFIED BRIEFING: LIVE CTF 0x05                               |
|                                                                 |
|NATIONAL SECURITY AGENCY / GMU RHET                              |
+-----------------------------------------------------------------+
```

## 1. The Intercept
Adversarial forces have deployed ground-based C-UAS laser dazzlers that track and blind friendly optical drones. The dazzlers are dynamically configured in the field using an infrared (IR) remote control running a customized NEC protocol.

## 2. The Mandate
We have captured an intact dazzler control board. The firmware uses deeply nested, dynamic conditional logic (`if / else if / switch`) to parse the raw hex codes from the IR receiver. There is a "Friendly Fire Lockout" code buried deep in the dynamic conditionals that permanently disables the dazzler.

You must step through the dynamic branches in GDB, trace the `CMP` and `BNE` chains, and reverse engineer the exact 32-bit NEC hex code required to trigger the lockout state.

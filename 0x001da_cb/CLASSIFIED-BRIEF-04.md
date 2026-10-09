# Operation Broken Wing: Classified Intelligence Briefing

```
+-----------------------------------------------------------------+
|                       TOP SECRET // NOFORN                      |
+-----------------------------------------------------------------+
|                                                                 |
|OPERATION BROKEN WING                                            |
|                                                                 |
|CLASSIFIED BRIEFING: LIVE CTF 0x04                               |
|                                                                 |
|NATIONAL SECURITY AGENCY / GMU RHET                              |
+-----------------------------------------------------------------+
```

## 1. The Intercept
A hostile heavy-lift UAS was brought down intact by a kinetic net. The airframe carries a chemical payload bay secured by a heavy-duty SG90 servo motor. The flight controller uses a static conditional state machine to ensure the payload only drops when the drone is explicitly in `STATE_ARMED` and above 500 meters. 

## 2. The Mandate
EOD (Explosive Ordnance Disposal) needs to safely trigger the servo to open the payload bay on the ground. The static conditional logic prevents the PWM signal from firing because the altitude reads zero. 

You must attach GDB or use Ghidra to locate the static conditional `CMP` instruction checking the altitude, and invert the logic (`BEQ` to `BNE`) so the payload drops immediately on the bench.

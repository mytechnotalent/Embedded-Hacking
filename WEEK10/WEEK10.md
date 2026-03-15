# Week 10: Conditionals in Embedded Systems: Debugging and Hacking Static & Dynamic Conditionals w/ SG90 Servo Motor PWM Basics

## 🎯 What You'll Learn This Week

By the end of this tutorial, you will be able to:
- Understand the difference between static and dynamic conditionals in C
- Know how if/else statements and switch/case blocks work at the assembly level
- Understand Pulse Width Modulation (PWM) and how it controls servo motors
- Calculate PWM timing from system clock to servo pulse width
- Identify conditional branches in Ghidra (beq, bne instructions)
- Hack string literals and timing delays in binary files
- Modify branch targets to change program flow
- Create "stealth" functionality by NOP-ing out print statements
- Understand IEEE-754 floating-point for angle calculations

---

## 📚 Part 1: Understanding Conditionals in C

### What Are Conditionals?

**Conditionals** are programming structures that make decisions. They let your program choose different paths based on whether a condition is true or false. Think of them like a fork in the road - the program checks a condition and decides which way to go.

### Two Types of Conditionals

| Type        | Description                                    | Example                                           |
| ----------- | ---------------------------------------------- | ------------------------------------------------- |
| **Static**  | Condition value is known/fixed at compile time | `if (choice == 1)` where choice never changes     |
| **Dynamic** | Condition value changes based on runtime input | `if (choice == getchar())` where user types input |

---

## 📚 Part 2: Static Conditionals

### What Makes a Conditional "Static"?

A **static conditional** is one where the outcome is predetermined because the condition variable never changes during program execution:

```c
int choice = 1;  // This NEVER changes!

while (true) {
    if (choice == 1) {
        printf("1\r\n");      // This ALWAYS runs
    } else if (choice == 2) {
        printf("2\r\n");      // This NEVER runs
    } else {
        printf("?\r\n");      // This NEVER runs
    }
}
```

```
┌─────────────────────────────────────────────────────────────────┐
│  Static Conditional Flow                                        │
│                                                                 │
│  choice = 1 (set once, never changes)                           │
│       │                                                         │
│       ▼                                                         │
│  ┌─────────────┐                                                │
│  │ choice == 1 │────YES────► printf("1")                        │
│  └─────────────┘                                                │
│       │NO (never taken)                                         │
│       ▼                                                         │
│  ┌─────────────┐                                                │
│  │ choice == 2 │────YES────► printf("2") (never reached)        │
│  └─────────────┘                                                │
│       │NO                                                       │
│       ▼                                                         │
│  printf("?") (never reached)                                    │
│                                                                 │
│  The branching logic EXISTS but only ONE path ever executes!    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The if/else Statement

The `if/else` structure checks conditions in order:

```c
if (choice == 1) {
    // Do something if choice is 1
} else if (choice == 2) {
    // Do something if choice is 2
} else {
    // Do something for all other values
}
```

### The switch Statement

The `switch` statement is another way to handle multiple conditions:

```c
switch (choice) {
    case 1:
        printf("one\r\n");
        break;
    case 2:
        printf("two\r\n");
        break;
    default:
        printf("??\r\n");
}
```

**Key Differences:**

| Feature          | if/else                 | switch                     |
| ---------------- | ----------------------- | -------------------------- |
| **Condition**    | Any boolean expression  | Single variable comparison |
| **Values**       | Ranges, complex logic   | Discrete values only       |
| **Fall-through** | No                      | Yes (without `break`)      |
| **Readability**  | Good for 2-3 conditions | Better for many conditions |

---

## 📚 Part 3: Dynamic Conditionals

### What Makes a Conditional "Dynamic"?

A **dynamic conditional** is one where the condition variable changes based on runtime input:

```c
uint8_t choice = 0;

while (true) {
    choice = getchar();  // User types a key - VALUE CHANGES!
    
    if (choice == '1') {
        printf("1\r\n");
    } else if (choice == '2') {
        printf("2\r\n");
    } else {
        printf("??\r\n");
    }
}
```

```
┌─────────────────────────────────────────────────────────────────┐
│  Dynamic Conditional Flow                                       │
│                                                                 │
│  ┌────────────────┐                                             │
│  │ choice=getchar │◄──── User types 'x' on keyboard             │
│  └────────────────┘                                             │
│         │                                                       │
│         ▼                                                       │
│  ┌─────────────┐                                                │
│  │ choice=='1' │────YES────► printf("1"), move servo            │
│  └─────────────┘                                                │
│         │NO                                                     │
│         ▼                                                       │
│  ┌─────────────┐                                                │
│  │ choice=='2' │────YES────► printf("2"), move servo            │
│  └─────────────┘                                                │
│         │NO                                                     │
│         ▼                                                       │
│  printf("??")                                                   │
│         │                                                       │
│         └──────────────────► Loop back to getchar()             │
│                                                                 │
│  EACH iteration can take a DIFFERENT path!                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The getchar() Function

`getchar()` reads a single character from the serial terminal:

```c
uint8_t choice = getchar();  // Waits for user to type something
```

- Returns the ASCII value of the key pressed
- `'1'` = 0x31, `'2'` = 0x32, `'x'` = 0x78, `'y'` = 0x79
- Blocks (waits) until a key is pressed

---

## 📚 Part 4: Understanding PWM (Pulse Width Modulation)

### What is PWM?

**PWM** (Pulse Width Modulation) is a technique for controlling power by rapidly switching a signal on and off. The ratio of "on time" to "off time" determines the average power delivered.

```
┌─────────────────────────────────────────────────────────────────┐
│  PWM Signal - 50% Duty Cycle                                    │
│                                                                 │
│  HIGH ─┐     ┌─────┐     ┌─────┐     ┌─────┐                    │
│        │     │     │     │     │     │     │                    │
│  LOW   └─────┘     └─────┘     └─────┘     └─────                │
│        ◄──T──►                                                  │
│        ON  OFF                                                  │
│                                                                 │
│  Duty Cycle = ON time / Total period = 50%                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### PWM for Servo Control

Servo motors use PWM differently - they care about the **pulse width**, not the duty cycle percentage:

```
┌─────────────────────────────────────────────────────────────────┐
│  Servo PWM Signal (50 Hz = 20ms period)                         │
│                                                                 │
│  0° Position (1ms pulse):                                       │
│  HIGH ─┐                                                        │
│        │ 1ms                                                    │
│  LOW   └────────────────────────────── (19ms) ──────────        │
│        ◄────────────── 20ms ─────────────────────────►          │
│                                                                 │
│  90° Position (1.5ms pulse):                                    │
│  HIGH ─────┐                                                    │
│            │ 1.5ms                                              │
│  LOW       └─────────────────────────── (18.5ms) ───────        │
│                                                                 │
│  180° Position (2ms pulse):                                     │
│  HIGH ─────────┐                                                │
│                │ 2ms                                            │
│  LOW           └───────────────────────── (18ms) ───────        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The Magic Numbers

| Angle | Pulse Width | PWM Ticks (at 1MHz) |
| ----- | ----------- | ------------------- |
| 0°    | 1000 µs     | 1000                |
| 90°   | 1500 µs     | 1500                |
| 180°  | 2000 µs     | 2000                |

---

## 📚 Part 5: PWM Timing Calculations

### From 150 MHz to 50 Hz

The RP2350's system clock runs at **150 MHz** (150 million cycles per second). A servo needs a **50 Hz** signal (one pulse every 20 ms). How do we bridge this gap?

```
┌─────────────────────────────────────────────────────────────────┐
│  Clock Division Chain                                           │
│                                                                 │
│  System Clock: 150 MHz                                          │
│        │                                                        │
│        │ ÷ 150 (clock divider)                                  │
│        ▼                                                        │
│  PWM Tick Rate: 1 MHz (1 tick = 1 microsecond)                  │
│        │                                                        │
│        │ Count to 20,000 (wrap value = 19,999)                  │
│        ▼                                                        │
│  PWM Frequency: 50 Hz (20 ms period)                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The Math

**Step 1: Clock Division**
```
PWM Tick Rate = System Clock ÷ Divider
1,000,000 Hz = 150,000,000 Hz ÷ 150
```

**Step 2: Frame Period**
```
Period = (Wrap Value + 1) × Tick Duration
20 ms = 20,000 ticks × 1 µs/tick
```

**Step 3: Pulse Width to Ticks**
```
Ticks = Pulse Width (µs) × 1 tick/µs
1500 ticks = 1500 µs × 1
```

### Worked Example: 90° Angle

Let's calculate what happens when we command 90°:

1. **Angle to Pulse Width:**
   ```
   Pulse = MIN + (angle/180) × (MAX - MIN)
   Pulse = 1000 + (90/180) × (2000 - 1000)
   Pulse = 1000 + 0.5 × 1000
   Pulse = 1500 µs
   ```

2. **Pulse to PWM Ticks:**
   ```
   Level = 1500 µs × 1 tick/µs = 1500 ticks
   ```

3. **Hardware Timing:**
   - Signal HIGH for 1500 ticks (1.5 ms)
   - Signal LOW for 18,500 ticks (18.5 ms)
   - Total period: 20,000 ticks (20 ms)

---

## 📚 Part 6: Understanding the SG90 Servo Motor

### What is the SG90?

The **SG90** is a small, inexpensive hobby servo motor commonly used in robotics projects:

```
┌─────────────────────────────────────────────────────────────────┐
│  SG90 Servo Motor                                               │
│                                                                 │
│     ┌───────────────┐                                           │
│     │    ┌─────┐    │                                           │
│     │    │ ARM │    │ ◄── Rotates 0° to 180°                    │
│     │    └──┬──┘    │                                           │
│     │       │       │                                           │
│     │  ┌────┴────┐  │                                           │
│     │  │  MOTOR  │  │                                           │
│     │  │  GEAR   │  │                                           │
│     │  │  BOX    │  │                                           │
│     │  └─────────┘  │                                           │
│     └───────┬───────┘                                           │
│             │                                                   │
│     ┌───────┼───────┐                                           │
│     │       │       │                                           │
│   ORANGE  RED    BROWN                                          │
│   Signal  VCC    GND                                            │
│   (PWM)   (5V)                                                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### SG90 Specifications

| Parameter         | Value                     |
| ----------------- | ------------------------- |
| **Voltage**       | 4.8V - 6V (typically 5V)  |
| **Rotation**      | 0° to 180°                |
| **Pulse Width**   | 1000µs - 2000µs           |
| **Frequency**     | 50 Hz (20ms period)       |
| **Stall Current** | ~650mA (can spike to 1A+) |

### Wire Colors

| Wire Color | Function | Connect To      |
| ---------- | -------- | --------------- |
| **Brown**  | GND      | Ground          |
| **Red**    | VCC      | 5V Power (VBUS) |
| **Orange** | Signal   | GPIO Pin (PWM)  |

---

## 📚 Part 7: Power Supply Safety

### ⚠️ CRITICAL WARNING ⚠️

**NEVER power the servo directly from the Pico's 3.3V pin!**

Servos can draw over 1000mA during movement spikes. The Pico's 3.3V regulator cannot handle this and you will:
- Cause brownouts (Pico resets)
- Damage the Pico's voltage regulator
- Potentially damage your USB port

### Correct Power Setup

```
┌─────────────────────────────────────────────────────────────────┐
│  CORRECT Power Wiring                                           │
│                                                                 │
│  USB ────► VBUS (5V) ───┬──► Servo VCC (Red)                    │
│                         │                                       │
│                         └──► Capacitor (+)                      │
│                              │                                  │
│  Pico GND ──────────────┬────┴──► Capacitor (-)                 │
│                         │                                       │
│                         └──────► Servo GND (Brown)              │
│                                                                 │
│  Pico GPIO 6 ──────────────────► Servo Signal (Orange)          │
│                                                                 │
│  IMPORTANT: Use a 1000µF 25V capacitor across the servo         │
│  power to absorb current spikes!                                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Why the Capacitor?

The **1000µF capacitor** acts as a tiny battery:
- Absorbs sudden current demands when servo moves
- Prevents voltage drops that could reset the Pico
- Smooths out electrical noise

---

## 📚 Part 8: Setting Up Your Environment

### Prerequisites

Before we start, make sure you have:
1. A Raspberry Pi Pico 2 board
2. A Raspberry Pi Pico Debug Probe
3. Ghidra installed (for static analysis)
4. Python installed (for UF2 conversion)
5. A serial monitor (PuTTY, minicom, or screen)
6. An SG90 servo motor
7. A 1000µF 25V capacitor
8. The sample projects: `0x001d_static-conditionals` and `0x0020_dynamic-conditionals`

### Hardware Setup

Connect your servo like this:

| Servo Wire      | Pico 2 Pin |
| --------------- | ---------- |
| Brown (GND)     | GND        |
| Red (VCC)       | VBUS (5V)  |
| Orange (Signal) | GPIO 6     |

```
┌─────────────────────────────────────────────────────────────────┐
│  Servo Wiring with Capacitor                                    │
│                                                                 │
│  Pico 2                              SG90 Servo                 │
│  ┌──────────┐                       ┌──────────┐                │
│  │          │                       │          │                │
│  │ GPIO 6   │─────── Orange ───────►│ Signal   │                │
│  │          │                       │          │                │
│  │ VBUS(5V) │───┬─── Red ──────────►│ VCC      │                │
│  │          │   │                   │          │                │
│  │ GND      │───┼─── Brown ────────►│ GND      │                │
│  │          │   │                   └──────────┘                │
│  └──────────┘   │                                               │
│                 │    ┌─────────┐                                │
│                 └────┤ + CAP - ├──── GND                        │
│                      │ 1000µF │                                 │
│                      │  25V   │                                 │
│                      └─────────┘                                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Project Structure

```
Embedded-Hacking/
├── 0x001d_static-conditionals/
│   ├── build/
│   │   ├── 0x001d_static-conditionals.uf2
│   │   └── 0x001d_static-conditionals.bin
│   ├── main/
│   │   └── 0x001d_static-conditionals.c
│   └── servo.h
├── 0x0020_dynamic-conditionals/
│   ├── build/
│   │   ├── 0x0020_dynamic-conditionals.uf2
│   │   └── 0x0020_dynamic-conditionals.bin
│   ├── main/
│   │   └── 0x0020_dynamic-conditionals.c
│   └── servo.h
└── uf2conv.py
```

---

## 🔬 Part 9: Hands-On Tutorial - Static Conditionals Code

### Step 1: Review the Source Code

Let's examine the static conditionals code:

**File: `0x001d_static-conditionals.c`**

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.h"

#define SERVO_GPIO 6

int main(void) {
    stdio_init_all();
    
    int choice = 1;  // STATIC - never changes!
    
    servo_init(SERVO_GPIO);
    
    while (true) {
        // if/else conditional
        if (choice == 1) {
            printf("1\r\n");
        } else if (choice == 2) {
            printf("2\r\n");
        } else {
            printf("?\r\n");
        }
        
        // switch/case conditional
        switch (choice) {
            case 1:
                printf("one\r\n");
                break;
            case 2:
                printf("two\r\n");
                break;
            default:
                printf("??\r\n");
        }
        
        // Servo movement
        servo_set_angle(0.0f);
        sleep_ms(500);
        servo_set_angle(180.0f);
        sleep_ms(500);
    }
}
```

### Step 2: Understand the Program Flow

Since `choice = 1` and NEVER changes:

```
┌─────────────────────────────────────────────────────────────────┐
│  Static Conditional Execution                                   │
│                                                                 │
│  Every loop iteration:                                          │
│                                                                 │
│  1. Check if (choice == 1) → TRUE → print "1"                   │
│  2. Check switch case 1 → MATCH → print "one"                   │
│  3. Move servo to 0°                                            │
│  4. Wait 500ms                                                  │
│  5. Move servo to 180°                                          │
│  6. Wait 500ms                                                  │
│  7. Repeat forever...                                           │
│                                                                 │
│  Output always: "1" then "one" (forever)                        │
│  Servo: sweeps 0° → 180° → 0° → 180° (forever)                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Step 3: Flash the Binary to Your Pico 2

1. Hold the BOOTSEL button on your Pico 2
2. Plug in the USB cable (while holding BOOTSEL)
3. Release BOOTSEL - a drive called "RPI-RP2" appears
4. Drag and drop `0x001d_static-conditionals.uf2` onto the drive
5. The Pico will reboot and start running!

### Step 4: Verify It's Working

**Check the serial monitor (PuTTY at 115200 baud):**
```
1
one
1
one
1
one
...
```

**Watch the servo:**
- It should sweep from 0° to 180° every second
- The movement is continuous and repetitive

---

## 🔬 Part 10: Debugging with GDB (Static Conditionals)

### Step 5: Start OpenOCD (Terminal 1)

Open a terminal and start OpenOCD:

```powershell
openocd ^
  -s "C:\Users\flare-vm\.pico-sdk\openocd\0.12.0+dev\scripts" ^
  -f interface/cmsis-dap.cfg ^
  -f target/rp2350.cfg ^
  -c "adapter speed 5000"
```

You should see output indicating OpenOCD connected successfully to your Pico 2 via the Debug Probe.

### Step 6: Start GDB (Terminal 2)

Open a **new terminal** and launch GDB with the binary:

```powershell
arm-none-eabi-gdb build\0x001d_static-conditionals.elf
```

### Step 7: Connect to the Remote Target

In GDB, connect to OpenOCD:

```gdb
target remote :3333
```

### Step 8: Halt the Running Binary

Stop the processor:

```gdb
monitor halt
```

### Step 9: Examine Main Function

Disassemble around main to see the conditionals:

```gdb
disassemble 0x10000234,+200
```

Look for comparison and branch instructions that implement the if/else and switch logic.

### Step 10: Set a Breakpoint at Main

```gdb
break *0x10000234
```

Reset and run to hit the breakpoint:

```gdb
monitor reset halt
continue
```

### Step 11: Find the Comparison Instructions

Step through and examine the comparison:

```gdb
stepi 20
info registers
```

Look for `cmp` instructions that compare the `choice` variable (value 1).

### Step 12: Examine the Printf Arguments

Before printf calls, check r0 for the format string address:

```gdb
x/s $r0
```

You should see strings like `"1\r\n"` or `"one\r\n"`.

### Step 13: Watch the Servo Commands

Set a breakpoint on servo_set_angle:

```gdb
break *0x10000280
continue
```

Check the floating-point register for the angle:

```gdb
info registers s0
```

### Step 14: Examine the Timing Delay

Set a breakpoint on sleep_ms and check the delay value:

```gdb
break *0x10000290
continue
info registers r0
```

You should see `0x1f4` (500 decimal) for the 500ms delay.

### Step 15: Step Through the Loop

Watch one complete iteration:

```gdb
stepi 100
info registers
```

Notice how the static conditional always takes the same branch path.

### Step 16: Exit GDB

When done exploring:

```gdb
quit
```

---

## 🔬 Part 11: Setting Up Ghidra for Static Conditionals

### Step 17: Start Ghidra

Open a terminal and type:

```powershell
ghidraRun
```

### Step 18: Create a New Project

1. Click **File** → **New Project**
2. Select **Non-Shared Project**
3. Click **Next**
4. Enter Project Name: `0x001d_static-conditionals`
5. Click **Finish**

### Step 19: Import the Binary

1. Open your file explorer
2. Navigate to the `0x001d_static-conditionals/build/` folder
3. **Drag and drop** the `.bin` file into Ghidra's project window

### Step 20: Configure the Binary Format

**Click the three dots (…) next to "Language" and:**
1. Search for "Cortex"
2. Select **ARM Cortex 32 little endian default**
3. Click **OK**

**Click the "Options…" button and:**
1. Change **Block Name** to `.text`
2. Change **Base Address** to `10000000`
3. Click **OK**

### Step 21: Analyze the Binary

1. Double-click on the file in the project window
2. A dialog asks "Analyze now?" - Click **Yes**
3. Use default analysis options and click **Analyze**

Wait for analysis to complete.

---

## 🔬 Part 12: Resolving Functions in Ghidra (Static)

### Step 22: Navigate to Main

1. Press `G` (Go to address) and type `10000234`
2. Right-click → **Edit Function Signature**
3. Change to: `int main(void)`
4. Click **OK**

### Step 23: Resolve stdio_init_all

At address `0x10000236`:

1. Double-click on the called function
2. Right-click → **Edit Function Signature**
3. Change to: `bool stdio_init_all(void)`
4. Click **OK**

### Step 24: Resolve servo_init

Look for a function call where `r0` is loaded with `0x6` (GPIO pin 6):

```assembly
movs r0, #0x6      ; GPIO pin 6
bl   FUN_xxxxx     ; servo_init
```

1. Right-click → **Edit Function Signature**
2. Change to: `void servo_init(uint pin)`
3. Click **OK**

### Step 25: Resolve puts

Look for function calls that load string addresses into `r0`:

```assembly
ldr  r0, =0x10001c54   ; Address of "1" string
bl   FUN_xxxxx          ; puts
```

**How do we know it's puts?**
- It takes a single string argument
- The hex `0x31` is ASCII "1"
- The hex `0x0d` is carriage return "\r"
- We see "1" echoed in PuTTY

1. Right-click → **Edit Function Signature**
2. Change to: `int puts(char *s)`
3. Click **OK**

### Step 26: Resolve servo_set_angle

Look for a function that loads float constants. Inside the function, you'll find:
- `0x7D0` (2000 decimal) - maximum pulse width
- `0x3E8` (1000 decimal) - minimum pulse width

These are the servo pulse limits!

1. Right-click → **Edit Function Signature**
2. Change to: `void servo_set_angle(float degrees)`
3. Click **OK**

### Step 27: Resolve sleep_ms

Look for a function where `r0` is loaded with `0x1f4` (500 decimal):

```assembly
ldr  r0, =0x1f4    ; 500 milliseconds
bl   FUN_xxxxx     ; sleep_ms
```

1. Right-click → **Edit Function Signature**
2. Change to: `void sleep_ms(uint ms)`
3. Click **OK**

---

## 🔬 Part 13: Hacking Static Conditionals

### Step 28: Open the Bytes Editor

1. Click **Window** → **Bytes**
2. A new panel appears showing raw hex bytes
3. Click the pencil icon to enable editing

### Step 29: Hack #1 - Change "1" to "2"

Find the string "1" in memory:

1. Double-click on the address reference for "1" (around `0x10001c54`)
2. You'll see `31` (ASCII for "1")
3. Change `31` to `32` (ASCII for "2")

### Step 30: Hack #2 - Change "one" to "fun"

Find the string "one":

1. Navigate to the "one" string address
2. Find bytes `6f 6e 65` ("one" in ASCII)
3. Change to `66 75 6e` ("fun" in ASCII)

**ASCII Reference:**
| Character | Hex  |
| --------- | ---- |
| o         | 0x6f |
| n         | 0x6e |
| e         | 0x65 |
| f         | 0x66 |
| u         | 0x75 |
| n         | 0x6e |

### Step 31: Hack #3 - Speed Up the Servo

Find the sleep_ms delay value:

1. Look for `0x1f4` (500) in the code
2. This appears TWICE (once for each sleep_ms call)
3. Change both from `f4 01` to `64 00` (100 in little-endian)

**Before:** 500ms delay (servo moves slowly)
**After:** 100ms delay (servo moves FAST!)

### Step 32: Export and Flash

1. Click **File** → **Export Program**
2. Set **Format** to **Binary**
3. Name: `0x001d_static-conditionals-h.bin`
4. Click **OK**

Convert and flash:

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x001d_static-conditionals
python ..\uf2conv.py build\0x001d_static-conditionals-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

### Step 33: Verify the Hacks

**Serial output now shows:**
```
2
fun
2
fun
...
```

**The servo now moves 5x faster!** It's spinning like crazy!

---

## 🔬 Part 14: Dynamic Conditionals - The Source Code

### Step 34: Review the Dynamic Code

**File: `0x0020_dynamic-conditionals.c`**

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.h"

#define SERVO_GPIO 6

int main(void) {
    stdio_init_all();
    
    uint8_t choice = 0;  // DYNAMIC - changes with user input!
    
    servo_init(SERVO_GPIO);
    
    while (true) {
        choice = getchar();  // Wait for keyboard input
        
        if (choice == 0x31) {       // '1'
            printf("1\r\n");
        } else if (choice == 0x32) { // '2'
            printf("2\r\n");
        } else {
            printf("??\r\n");
        }
        
        switch (choice) {
            case '1':
                printf("one\r\n");
                servo_set_angle(0.0f);
                sleep_ms(500);
                servo_set_angle(180.0f);
                sleep_ms(500);
                break;
            case '2':
                printf("two\r\n");
                servo_set_angle(180.0f);
                sleep_ms(500);
                servo_set_angle(0.0f);
                sleep_ms(500);
                break;
            default:
                printf("??\r\n");
        }
    }
}
```

### Step 35: Understand the Dynamic Behavior

| User Types    | Output      | Servo Action |
| ------------- | ----------- | ------------ |
| '1' (0x31)    | "1" + "one" | 0° → 180°    |
| '2' (0x32)    | "2" + "two" | 180° → 0°    |
| Anything else | "??" + "??" | No movement  |

### Step 36: Flash and Test

1. Flash `0x0020_dynamic-conditionals.uf2`
2. Open PuTTY
3. Press '1' - servo sweeps one direction
4. Press '2' - servo sweeps the other direction
5. Press 'x' - prints "??" and no movement

---

## 🔬 Part 15: Debugging with GDB (Dynamic Conditionals)

### Step 37: Start OpenOCD (Terminal 1)

Open a terminal and start OpenOCD:

```powershell
openocd ^
  -s "C:\Users\flare-vm\.pico-sdk\openocd\0.12.0+dev\scripts" ^
  -f interface/cmsis-dap.cfg ^
  -f target/rp2350.cfg ^
  -c "adapter speed 5000"
```

You should see output indicating OpenOCD connected successfully to your Pico 2 via the Debug Probe.

### Step 38: Start GDB (Terminal 2)

Open a **new terminal** and launch GDB with the binary:

```powershell
arm-none-eabi-gdb build\0x0020_dynamic-conditionals.elf
```

### Step 39: Connect to the Remote Target

In GDB, connect to OpenOCD:

```gdb
target remote :3333
```

### Step 40: Halt the Running Binary

Stop the processor:

```gdb
monitor halt
```

### Step 41: Examine Main Function

Disassemble around main to see the dynamic conditionals:

```gdb
disassemble 0x10000234,+250
```

Look for the `getchar` call followed by comparison and branch instructions.

### Step 42: Set a Breakpoint After getchar

Find where `choice` gets its value:

```gdb
break *0x10000250
```

Reset and continue:

```gdb
monitor reset halt
continue
```

### Step 43: Watch the Input Value

When you press a key in PuTTY, the breakpoint hits. Check the return value:

```gdb
info registers r0
```

If you pressed '1', you should see `0x31`. If you pressed '2', you should see `0x32`.

### Step 44: Trace the Comparison Logic

Step through the comparison instructions:

```gdb
stepi 10
info registers
```

Watch for `cmp r0, #0x31` and `cmp r0, #0x32` instructions.

### Step 45: Examine the Branch Decisions

Look at the condition flags after comparison:

```gdb
info registers cpsr
```

The zero flag (Z) determines if the branch is taken.

### Step 46: Watch Different Input Paths

Continue and press different keys to see how the program takes different branches:

```gdb
continue
```

Press '2' in PuTTY, then examine registers again.

### Step 47: Examine Servo Control

Set a breakpoint on servo_set_angle:

```gdb
break *0x10000280
continue
```

Check the angle value:

```gdb
info registers s0
```

### Step 48: Exit GDB

When done exploring:

```gdb
quit
```

---

## 🔬 Part 16: Setting Up Ghidra for Dynamic Conditionals

### Step 49: Create New Project

1. Create project: `0x0020_dynamic-conditionals`
2. Import the `.bin` file
3. Configure as ARM Cortex, base address `10000000`
4. Analyze

### Step 50: Navigate to Main

Press `G` and go to `10000234`.

### Step 51: Resolve Functions

Follow the same process:

1. **main** at `0x10000234` → `int main(void)`
2. **stdio_init_all** → `bool stdio_init_all(void)`
3. **servo_init** → `void servo_init(uint pin)`
4. **puts** → `int puts(char *s)`
5. **servo_set_angle** → `void servo_set_angle(float degrees)`
6. **sleep_ms** → `void sleep_ms(uint ms)`

### Step 52: Identify getchar

Look for a function that:
- Returns a value in `r0`
- That value is then compared against `0x31` ("1")

```assembly
bl   FUN_xxxxx      ; This is getchar!
mov  r4, r0         ; Save return value
cmp  r4, #0x31      ; Compare to '1'
beq  LAB_xxxxx      ; Branch if equal
```

1. Right-click → **Edit Function Signature**
2. Change to: `int getchar(void)`
3. Click **OK**

### Step 53: Identify Hardware Addresses

Double-click into `stdio_init_all` and look for hardware addresses:

```assembly
ldr  r0, =0x40070000   ; UART0 base address
```

Check the RP2350 datasheet Section 2.2 (Address Map):
- `0x40070000` = UART0

This confirms it's a UART initialization function!

---

## 🔬 Part 17: Understanding Branch Instructions

### ARM Branch Instructions

| Instruction | Meaning                | Condition          |
| ----------- | ---------------------- | ------------------ |
| `b`         | Branch (always)        | Unconditional jump |
| `beq`       | Branch if Equal        | Zero flag set      |
| `bne`       | Branch if Not Equal    | Zero flag clear    |
| `bgt`       | Branch if Greater Than | Signed greater     |
| `blt`       | Branch if Less Than    | Signed less        |

### How Conditionals Become Branches

```c
if (choice == 0x31) {
    printf("1");
}
```

Becomes:

```assembly
cmp  r4, #0x31      ; Compare choice to '1'
bne  skip_printf    ; If NOT equal, skip the printf
; ... printf code here ...
skip_printf:
```

```
┌─────────────────────────────────────────────────────────────────┐
│  Conditional Branch Flow                                        │
│                                                                 │
│  cmp r4, #0x31                                                  │
│       │                                                         │
│       │ (Sets flags based on r4 - 0x31)                         │
│       ▼                                                         │
│  beq target_address                                             │
│       │                                                         │
│       ├── If r4 == 0x31: Jump to target_address                 │
│       │                                                         │
│       └── If r4 != 0x31: Continue to next instruction           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔬 Part 18: Advanced Hacking - Creating Stealth Commands

### The Goal

We want to create **secret commands** that:
1. Respond to 'x' and 'y' instead of '1' and '2'
2. Move the servo WITHOUT printing anything
3. Leave NO trace in the terminal

### Step 54: Plan the Patches

**Original behavior:**
- '1' (0x31) → prints "1" and "one", moves servo
- '2' (0x32) → prints "2" and "two", moves servo

**Hacked behavior:**
- 'x' (0x78) → moves servo SILENTLY
- 'y' (0x79) → moves servo SILENTLY
- '1' (0x31) → prints "1" and "one" (normal)
- '2' (0x32) → prints "2" and "two" (normal)

### Step 55: Change Comparison Values

Find the compare instructions:

```assembly
cmp  r4, #0x31      ; Change to #0x78 ('x')
...
cmp  r4, #0x32      ; Change to #0x79 ('y')
```

### Step 56: Redirect Branches to Skip Prints

For the stealth keys, we need to jump PAST the printf calls directly to the servo code.

**Original flow:**
```
compare → branch → printf("1") → printf("one") → servo code
```

**Hacked flow:**
```
compare 'x' → branch → [skip prints] → servo code
```

Change the `beq` target addresses:
- Original: `beq 0x10000270` (goes to printf)
- Hacked: `beq 0x1000027c` (skips to servo)

### Step 57: NOP Out Print Calls

**NOP** (No Operation) is an instruction that does nothing. We use it to "erase" code without changing the size of the binary.

ARM Thumb NOP encoding: `00 bf` (2 bytes)

To NOP out a `bl puts` instruction (4 bytes), use `00 bf 00 bf`.

### Step 58: Apply All Patches

Here's the complete patch list:

| Offset   | Original     | Patched      | Purpose                      |
| -------- | ------------ | ------------ | ---------------------------- |
| Compare1 | `31`         | `78`         | Check for 'x' instead of '1' |
| Compare2 | `32`         | `79`         | Check for 'y' instead of '2' |
| Branch1  | target=0x270 | target=0x27c | Skip printf for 'x'          |
| Branch2  | target=0x29a | target=0x2a6 | Skip printf for 'y'          |
| puts1    | `bl puts`    | `nop nop`    | Remove print                 |
| puts2    | `bl puts`    | `nop nop`    | Remove print                 |

### Step 59: Hack the Angle Value

Let's also change 180° to 30° for fun!

**Original:** `0x43340000` (180.0f in IEEE-754)
**New:** `0x41f00000` (30.0f in IEEE-754)

**Calculation for 30.0f:**
```
30.0 = 1.875 × 2^4
Sign = 0
Exponent = 127 + 4 = 131 = 0x83
Mantissa = 0.875 = 0x700000

Binary: 0 10000011 11100000000000000000000
Hex: 0x41f00000
Little-endian: 00 00 f0 41
```

### Step 60: Export and Test

1. Export as `0x0020_dynamic-conditionals-h.bin`
2. Convert to UF2:

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0020_dynamic-conditionals
python ..\uf2conv.py build\0x0020_dynamic-conditionals-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

3. Flash and test:
   - Press '1' → prints "1" and "one", servo moves
   - Press '2' → prints "2" and "two", servo moves
   - Press 'x' → NO OUTPUT, but servo moves silently!
   - Press 'y' → NO OUTPUT, but servo moves silently!

---

## 📊 Part 19: Summary and Review

### What We Accomplished

1. **Learned static vs dynamic conditionals** - Fixed vs runtime-determined values
2. **Understood if/else and switch/case** - Two ways to branch in C
3. **Mastered PWM calculations** - 150MHz to 50Hz servo signal
4. **Identified conditional branches in assembly** - beq, bne, cmp instructions
5. **Hacked string literals** - Changed "one" to "fun"
6. **Modified timing values** - Sped up servo from 500ms to 100ms
7. **Created stealth commands** - Hidden 'x' and 'y' keys
8. **NOPed out print statements** - Removed logging for stealth
9. **Redirected branch targets** - Changed program flow

### Static vs Dynamic Summary

```
┌─────────────────────────────────────────────────────────────────┐
│  Static Conditionals                                            │
│  ───────────────────                                            │
│  • Variable set once, never changes                             │
│  • Same path taken every iteration                              │
│  • Compiler may optimize out dead branches                      │
│  • Example: int choice = 1; if (choice == 1)                    │
├─────────────────────────────────────────────────────────────────┤
│  Dynamic Conditionals                                           │
│  ────────────────────                                           │
│  • Variable changes based on input/sensors                      │
│  • Different paths taken based on runtime state                 │
│  • All branches must remain in binary                           │
│  • Example: choice = getchar(); if (choice == '1')              │
└─────────────────────────────────────────────────────────────────┘
```

### PWM Calculation Summary

```
┌─────────────────────────────────────────────────────────────────┐
│  Servo PWM Calculation Chain                                    │
│                                                                 │
│  Angle (degrees) → Pulse Width (µs) → PWM Ticks → Servo Motion  │
│                                                                 │
│  0°   → 1000 µs → 1000 ticks → Fully counter-clockwise          │
│  90°  → 1500 µs → 1500 ticks → Center position                  │
│  180° → 2000 µs → 2000 ticks → Fully clockwise                  │
│                                                                 │
│  Formula: pulse = 1000 + (angle/180) × 1000                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Key Memory Addresses

| Memory Address | Description              |
| -------------- | ------------------------ |
| `0x10000234`   | main() function          |
| `0x40070000`   | UART0 hardware registers |
| `0x1f4`        | 500 (sleep_ms delay)     |
| `0x7D0`        | 2000 (max pulse width)   |
| `0x3E8`        | 1000 (min pulse width)   |
| `0x43340000`   | 180.0f (max angle)       |

---

## ✅ Practice Exercises

### Exercise 1: Change Servo Angle Range
Modify the servo to sweep from 45° to 135° instead of 0° to 180°.

**Hint:** Calculate IEEE-754 values for 45.0f and 135.0f.

### Exercise 2: Add a Third Command
Add support for key '3' that moves the servo to 90° (center position).

**Hint:** You'll need to find space in the binary or modify existing code.

### Exercise 3: Reverse the Servo Direction
Make '1' do what '2' does and vice versa.

**Hint:** Swap the branch targets.

### Exercise 4: Speed Profile
Create fast movement for '1' (100ms) and slow movement for '2' (1000ms).

**Hint:** Find both sleep_ms calls and patch them differently.

### Exercise 5: Complete Stealth Mode
Make ALL servo movements silent - remove ALL printf and puts calls.

**Hint:** NOP out every output function call.

---

## 🎓 Key Takeaways

1. **Static conditionals have fixed outcomes** - The same path always executes

2. **Dynamic conditionals respond to input** - Different paths based on runtime state

3. **PWM frequency = 50Hz for servos** - One pulse every 20ms

4. **Pulse width encodes position** - 1ms=0°, 1.5ms=90°, 2ms=180°

5. **beq = branch if equal** - Jumps when comparison matches

6. **bne = branch if not equal** - Jumps when comparison doesn't match

7. **NOP erases code without changing size** - `00 bf` in ARM Thumb

8. **Branch targets can be redirected** - Change where code jumps to

9. **IEEE-754 is needed for angles** - Floats have specific bit patterns

10. **Stealth requires removing ALL output** - NOP out printf AND puts

---

## 📖 Glossary

| Term                    | Definition                                          |
| ----------------------- | --------------------------------------------------- |
| **beq**                 | Branch if Equal - ARM conditional jump              |
| **bne**                 | Branch if Not Equal - ARM conditional jump          |
| **Dynamic Conditional** | Condition that changes based on runtime input       |
| **Duty Cycle**          | Percentage of time signal is HIGH                   |
| **getchar()**           | C function that reads one character from input      |
| **NOP**                 | No Operation - instruction that does nothing        |
| **PWM**                 | Pulse Width Modulation - variable duty cycle signal |
| **SG90**                | Common hobby servo motor model                      |
| **Static Conditional**  | Condition with fixed/predetermined outcome          |
| **switch/case**         | C structure for multiple discrete value comparisons |
| **Wrap Value**          | PWM counter maximum before reset                    |

---

## 🔗 Additional Resources

### ASCII Reference Table

| Character | Hex  | Decimal |
| --------- | ---- | ------- |
| '0'       | 0x30 | 48      |
| '1'       | 0x31 | 49      |
| '2'       | 0x32 | 50      |
| 'x'       | 0x78 | 120     |
| 'y'       | 0x79 | 121     |
| '\r'      | 0x0d | 13      |
| '\n'      | 0x0a | 10      |

### IEEE-754 Common Angles

| Angle | IEEE-754 Hex | Little-Endian Bytes |
| ----- | ------------ | ------------------- |
| 0.0   | 0x00000000   | 00 00 00 00         |
| 30.0  | 0x41f00000   | 00 00 f0 41         |
| 45.0  | 0x42340000   | 00 00 34 42         |
| 90.0  | 0x42b40000   | 00 00 b4 42         |
| 135.0 | 0x43070000   | 00 00 07 43         |
| 180.0 | 0x43340000   | 00 00 34 43         |

### ARM Thumb NOP Encodings

| Instruction | Encoding      | Size    |
| ----------- | ------------- | ------- |
| `nop`       | `00 bf`       | 2 bytes |
| `nop.w`     | `00 f0 00 80` | 4 bytes |

### RP2350 Key Addresses

| Address      | Peripheral |
| ------------ | ---------- |
| `0x40070000` | UART0      |
| `0x40078000` | UART1      |
| `0x40050000` | PWM        |

---

## 🚨 Real-World Implications

### Why Stealth Commands Matter

The ability to create hidden commands has serious implications:

**Legitimate Uses:**
- Factory test modes
- Debugging interfaces
- Emergency recovery features

**Malicious Uses:**
- Backdoors in firmware
- Hidden surveillance features
- Unauthorized control of systems

### Real-World Example

Imagine a drone with hacked firmware:
- Normal keys ('1', '2') control it visibly with logging
- Hidden keys ('x', 'y') control it with NO log entries
- An attacker could operate the drone while security monitors show nothing

### The Nuclear Fuel Rod Analogy

A fast-moving servo is like a nuclear fuel rod:
- Both are small components with immense power
- Both require precise control to prevent damage
- Both can "go critical" if pushed beyond limits
- Both teach the importance of safety margins

---

**Remember:** The techniques you learned today demonstrate how conditional logic can be manipulated at the binary level. Understanding these attacks helps us build more secure embedded systems. Always use your skills ethically and responsibly!

Happy hacking! 🔧

# Week 11: Structures and Functions in Embedded Systems: Debugging and Hacking w/ IR Remote Control and NEC Protocol Basics

## 🎯 What You'll Learn This Week

By the end of this tutorial, you will be able to:
- Understand C structures (structs) and how they organize related data
- Know how structs are represented in memory and assembly code
- Understand the NEC infrared (IR) protocol for remote control communication
- Create and use functions with parameters and return values
- Identify struct member access patterns in Ghidra
- Recognize how compilers "flatten" structs into individual operations
- Hack GPIO pin assignments to swap LED behavior
- Understand the security implications of log/behavior desynchronization
- Analyze .elf files in addition to .bin files in Ghidra

---

## 📚 Part 1: Understanding C Structures (Structs)

### What is a Struct?

A **structure** (or **struct**) is a user-defined data type that groups related variables together under one name. Think of it like a form with multiple fields - each field can hold different types of data, but they all belong together.

```c
// Define a struct type
typedef struct {
    uint8_t led1_pin;      // GPIO pin for LED 1
    uint8_t led2_pin;      // GPIO pin for LED 2
    uint8_t led3_pin;      // GPIO pin for LED 3
    bool led1_state;       // Is LED 1 on?
    bool led2_state;       // Is LED 2 on?
    bool led3_state;       // Is LED 3 on?
} simple_led_ctrl_t;
```

```
┌─────────────────────────────────────────────────────────────────┐
│  Structure as a Container                                       │
│                                                                 │
│  simple_led_ctrl_t leds                                         │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │  led1_pin: 16     led2_pin: 17     led3_pin: 18            ││
│  │  ┌────────┐       ┌────────┐       ┌────────┐              ││
│  │  │   16   │       │   17   │       │   18   │              ││
│  │  └────────┘       └────────┘       └────────┘              ││
│  │                                                             ││
│  │  led1_state: false  led2_state: false  led3_state: false   ││
│  │  ┌────────┐       ┌────────┐       ┌────────┐              ││
│  │  │ false  │       │ false  │       │ false  │              ││
│  │  └────────┘       └────────┘       └────────┘              ││
│  └─────────────────────────────────────────────────────────────┘│
│                                                                 │
│  All 6 members live together as ONE variable called "leds"      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Why Use Structs?

| Without Structs (Messy!)   | With Structs (Clean!)       |
| -------------------------- | --------------------------- |
| `uint8_t led1_pin = 16;`   | `simple_led_ctrl_t leds;`   |
| `uint8_t led2_pin = 17;`   | `leds.led1_pin = 16;`       |
| `uint8_t led3_pin = 18;`   | `leds.led2_pin = 17;`       |
| `bool led1_state = false;` | `leds.led3_pin = 18;`       |
| `bool led2_state = false;` | `leds.led1_state = false;`  |
| `bool led3_state = false;` | ... (all in one container!) |

**Benefits of Structs:**
1. **Organization** - Related data stays together
2. **Readability** - Code is easier to understand
3. **Maintainability** - Changes are easier to make
4. **Scalability** - Easy to add more LEDs or features
5. **Passing to Functions** - Pass one struct instead of many variables

---

## 📚 Part 2: Struct Memory Layout

### How Structs are Stored in Memory

When you create a struct, the compiler places each member in consecutive memory locations:

```
┌─────────────────────────────────────────────────────────────────┐
│  Memory Layout of simple_led_ctrl_t                             │
│                                                                 │
│  Address    Member          Size    Value                       │
│  ─────────────────────────────────────────────────────────────  │
│  0x2000000  led1_pin        1 byte  16 (0x10)                   │
│  0x2000001  led2_pin        1 byte  17 (0x11)                   │
│  0x2000002  led3_pin        1 byte  18 (0x12)                   │
│  0x2000003  led1_state      1 byte  0 (false)                   │
│  0x2000004  led2_state      1 byte  0 (false)                   │
│  0x2000005  led3_state      1 byte  0 (false)                   │
│                                                                 │
│  Total struct size: 6 bytes                                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Accessing Struct Members

Use the **dot operator** (`.`) to access members:

```c
simple_led_ctrl_t leds;

// Set values
leds.led1_pin = 16;
leds.led1_state = true;

// Read values
printf("Pin: %d\n", leds.led1_pin);
```

### Pointer to Struct (Arrow Operator)

When you have a **pointer** to a struct, use the **arrow operator** (`->`):

```c
simple_led_ctrl_t leds;
simple_led_ctrl_t *ptr = &leds;  // Pointer to the struct

// These are equivalent:
leds.led1_pin = 16;              // Using dot with struct variable
ptr->led1_pin = 16;              // Using arrow with pointer
(*ptr).led1_pin = 16;            // Dereferencing then dot (same thing)
```

```
┌─────────────────────────────────────────────────────────────────┐
│  Dot vs Arrow Operator                                          │
│                                                                 │
│  struct_variable.member    ◄── Use with actual struct           │
│                                                                 │
│  pointer_to_struct->member ◄── Use with pointer to struct       │
│                                                                 │
│  The arrow (->) is shorthand for (*pointer).member              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📚 Part 3: Designated Initializers

### Clean Struct Initialization

C allows you to initialize struct members by name using **designated initializers**:

```c
simple_led_ctrl_t leds = {
    .led1_pin = 16,
    .led2_pin = 17,
    .led3_pin = 18,
    .led1_state = false,
    .led2_state = false,
    .led3_state = false
};
```

**Benefits:**
- Clear which value goes to which member
- Order doesn't matter (can rearrange lines)
- Self-documenting code
- Easy to add new members later

---

## 📚 Part 4: Understanding the NEC IR Protocol

### What is Infrared (IR) Communication?

**Infrared** communication uses invisible light pulses to send data. Your TV remote uses IR to send commands to your TV. The LED in the remote flashes on and off very quickly in specific patterns that represent different buttons.

```
┌─────────────────────────────────────────────────────────────────┐
│  IR Communication                                               │
│                                                                 │
│  Remote Control                        IR Receiver              │
│  ┌──────────┐                         ┌──────────┐              │
│  │  Button  │                         │          │              │
│  │    1     │ ─── IR Light Pulses ──► │  ████    │              │
│  │  ┌───┐   │     ~~~~~~~~~~~~►       │  Sensor  │              │
│  │  │ ● │   │                         │          │              │
│  │  └───┘   │                         └────┬─────┘              │
│  │  IR LED  │                              │                    │
│  └──────────┘                              ▼                    │
│                                       GPIO Pin                  │
│                                       (Digital signal)          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The NEC Protocol

**NEC** is one of the most common IR protocols. When you press a button, the remote sends:

1. **Leader pulse** - 9ms HIGH, 4.5ms LOW (says "attention!")
2. **Address** - 8 bits identifying the device
3. **Address Inverse** - 8 bits (for error checking)
4. **Command** - 8 bits for the button pressed
5. **Command Inverse** - 8 bits (for error checking)

```
┌─────────────────────────────────────────────────────────────────┐
│  NEC Protocol Frame                                             │
│                                                                 │
│  ┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐  │
│  │ Leader  │ Address │ Address │ Command │ Command │  Stop   │  │
│  │  Pulse  │  8-bit  │ Inverse │  8-bit  │ Inverse │  Bit    │  │
│  │ 9+4.5ms │         │  8-bit  │         │  8-bit  │         │  │
│  └─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘  │
│                                                                 │
│  Total: 32 bits of data (+ leader + stop)                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### NEC Command Codes for Our Remote

| Button | NEC Command Code | Hex Value |
| ------ | ---------------- | --------- |
| 1      | 0x0C             | 12        |
| 2      | 0x18             | 24        |
| 3      | 0x5E             | 94        |

**Note:** Different remotes have different codes. These are specific to our example remote.

---

## 📚 Part 5: Understanding Functions in C

### What is a Function?

A **function** is a reusable block of code that performs a specific task. Functions help organize code and avoid repetition.

```c
// Function definition
int add_numbers(int a, int b) {
    return a + b;
}

// Function call
int result = add_numbers(5, 3);  // result = 8
```

### Function Components

```
┌─────────────────────────────────────────────────────────────────┐
│  Anatomy of a Function                                          │
│                                                                 │
│  return_type  function_name ( parameters ) {                    │
│      // function body                                           │
│      return value;                                              │
│  }                                                              │
│                                                                 │
│  Example:                                                       │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ int    ir_to_led_number ( int ir_command ) {                ││
│  │ ───    ───────────────   ───────────────                    ││
│  │  │           │                  │                           ││
│  │  │           │                  └── Parameter (input)       ││
│  │  │           └── Function name                              ││
│  │  └── Return type (what it gives back)                       ││
│  │                                                             ││
│  │     if (ir_command == 0x0C) return 1;  ◄── Body             ││
│  │     if (ir_command == 0x18) return 2;                       ││
│  │     return 0;                          ◄── Return value     ││
│  │ }                                                           ││
│  └─────────────────────────────────────────────────────────────┘│
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Types of Functions

| Type                         | Description               | Example                      |
| ---------------------------- | ------------------------- | ---------------------------- |
| **No params, no return**     | Just does something       | `void leds_all_off(void)`    |
| **With params, no return**   | Takes input, no output    | `void blink_led(pin, count)` |
| **No params, with return**   | No input, gives output    | `int ir_getkey(void)`        |
| **With params, with return** | Takes input, gives output | `int ir_to_led_number(cmd)`  |

---

## 📚 Part 6: Functions with Struct Pointers

### Passing Structs to Functions

When passing a struct to a function, you usually pass a **pointer** to avoid copying all the data:

```c
// Function takes a POINTER to the struct
void leds_all_off(simple_led_ctrl_t *leds) {
    gpio_put(leds->led1_pin, false);  // Use arrow operator!
    gpio_put(leds->led2_pin, false);
    gpio_put(leds->led3_pin, false);
}

// Call with address-of operator
simple_led_ctrl_t my_leds;
leds_all_off(&my_leds);  // Pass the ADDRESS of my_leds
```

```
┌─────────────────────────────────────────────────────────────────┐
│  Passing Struct by Pointer                                      │
│                                                                 │
│  main() {                                                       │
│      simple_led_ctrl_t leds;    ◄── Struct lives here           │
│      leds_all_off(&leds);       ◄── Pass ADDRESS (pointer)      │
│  }                                  │                           │
│                                     │                           │
│                                     ▼                           │
│  leds_all_off(simple_led_ctrl_t *leds) {                        │
│      gpio_put(leds->led1_pin, false);                           │
│              ────                                               │
│               │                                                 │
│               └── Arrow because leds is a POINTER               │
│  }                                                              │
│                                                                 │
│  WHY use pointers?                                              │
│  • Efficient: Only 4 bytes (address) instead of entire struct   │
│  • Allows modification: Function can change the original        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📚 Part 7: How Compilers Handle Structs

### Struct "Flattening" in Assembly

When the compiler converts your C code to assembly, it "flattens" struct operations into individual memory accesses:

**C Code:**
```c
gpio_init(leds.led1_pin);  // leds.led1_pin = 16
gpio_init(leds.led2_pin);  // leds.led2_pin = 17
gpio_init(leds.led3_pin);  // leds.led3_pin = 18
```

**Assembly (what the compiler produces):**
```assembly
movs r0, #0x10      ; r0 = 16 (led1_pin value)
bl   gpio_init      ; call gpio_init(16)

movs r0, #0x11      ; r0 = 17 (led2_pin value)
bl   gpio_init      ; call gpio_init(17)

movs r0, #0x12      ; r0 = 18 (led3_pin value)
bl   gpio_init      ; call gpio_init(18)
```

```
┌─────────────────────────────────────────────────────────────────┐
│  Struct Flattening                                              │
│                                                                 │
│  C Level (High-level abstraction):                              │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │  gpio_init(leds.led1_pin);                                  ││
│  │  gpio_init(leds.led2_pin);                                  ││
│  │  gpio_init(leds.led3_pin);                                  ││
│  └─────────────────────────────────────────────────────────────┘│
│                      │                                          │
│                      │ Compiler transforms                      │
│                      ▼                                          │
│  Assembly Level (Flattened):                                    │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │  movs r0, #16       ; Just the VALUE, no struct reference   ││
│  │  bl   gpio_init                                             ││
│  │  movs r0, #17       ; Next value directly                   ││
│  │  bl   gpio_init                                             ││
│  │  movs r0, #18       ; Next value directly                   ││
│  │  bl   gpio_init                                             ││
│  └─────────────────────────────────────────────────────────────┘│
│                                                                 │
│  The struct abstraction DISAPPEARS at the assembly level!       │
│  We just see individual values being loaded and used.           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Why This Matters for Reverse Engineering

- In Ghidra, you won't always see "struct" - just individual values
- You must recognize PATTERNS (sequential values like 16, 17, 18)
- Understanding flattening helps you reconstruct the original struct

---

## 📚 Part 8: Setting Up Your Environment

### Prerequisites

Before we start, make sure you have:
1. A Raspberry Pi Pico 2 board
2. A Raspberry Pi Pico Debug Probe
3. Ghidra installed (for static analysis)
4. Python installed (for UF2 conversion)
5. A serial monitor (PuTTY, minicom, or screen)
6. An IR receiver module (like VS1838B)
7. An IR remote control (any NEC-compatible remote)
8. Three LEDs (red, green, yellow) with resistors
9. The sample projects: `0x0023_structures` and `0x0026_functions`

### Hardware Setup

**IR Receiver Wiring:**

| IR Receiver Pin | Pico 2 Pin |
| --------------- | ---------- |
| VCC             | 3.3V       |
| GND             | GND        |
| OUT/DATA        | GPIO 5     |

**LED Wiring:**

| LED    | GPIO Pin | Resistor  |
| ------ | -------- | --------- |
| Red    | GPIO 16  | 220Ω-330Ω |
| Green  | GPIO 17  | 220Ω-330Ω |
| Yellow | GPIO 18  | 220Ω-330Ω |

```
┌─────────────────────────────────────────────────────────────────┐
│  Complete Wiring Diagram                                        │
│                                                                 │
│  Pico 2                     Components                          │
│  ┌──────────┐                                                   │
│  │          │              ┌─────────────┐                      │
│  │ GPIO 5   │──────────────┤ IR Receiver │                      │
│  │          │              │  (VS1838B)  │                      │
│  │          │              └──────┬──────┘                      │
│  │          │                     │                             │
│  │ GPIO 16  │───[220Ω]───(RED LED)────┐                         │
│  │          │                         │                         │
│  │ GPIO 17  │───[220Ω]───(GRN LED)────┤                         │
│  │          │                         │                         │
│  │ GPIO 18  │───[220Ω]───(YEL LED)────┤                         │
│  │          │                         │                         │
│  │ 3.3V     │─────────────────────────┼── IR VCC                │
│  │          │                         │                         │
│  │ GND      │─────────────────────────┴── All GNDs              │
│  │          │                                                   │
│  └──────────┘                                                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Project Structure

```
Embedded-Hacking/
├── 0x0023_structures/
│   ├── build/
│   │   ├── 0x0023_structures.uf2
│   │   └── 0x0023_structures.bin
│   ├── main/
│   │   └── 0x0023_structures.c
│   └── ir.h
├── 0x0026_functions/
│   ├── build/
│   │   ├── 0x0026_functions.uf2
│   │   ├── 0x0026_functions.bin
│   │   └── 0x0026_functions.elf
│   ├── main/
│   │   └── 0x0026_functions.c
│   └── ir.h
└── uf2conv.py
```

---

## 🔬 Part 9: Hands-On Tutorial - Structures Code

### Step 1: Review the Source Code

Let's examine the structures code:

**File: `0x0023_structures.c`**

```c
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "ir.h"

#define IR_PIN 5

typedef struct {
    uint8_t led1_pin;
    uint8_t led2_pin;
    uint8_t led3_pin;
    bool led1_state;
    bool led2_state;
    bool led3_state;
} simple_led_ctrl_t;

int main(void) {
    stdio_init_all();
    
    simple_led_ctrl_t leds = {
        .led1_pin = 16,
        .led2_pin = 17,
        .led3_pin = 18,
        .led1_state = false,
        .led2_state = false,
        .led3_state = false
    };
    
    gpio_init(leds.led1_pin); gpio_set_dir(leds.led1_pin, GPIO_OUT);
    gpio_init(leds.led2_pin); gpio_set_dir(leds.led2_pin, GPIO_OUT);
    gpio_init(leds.led3_pin); gpio_set_dir(leds.led3_pin, GPIO_OUT);
    
    ir_init(IR_PIN);
    printf("IR receiver on GPIO %d ready\n", IR_PIN);
    
    while (true) {
        int key = ir_getkey();
        if (key >= 0) {
            printf("NEC command: 0x%02X\n", key);
            
            // Turn all off first
            leds.led1_state = false;
            leds.led2_state = false;
            leds.led3_state = false;
            
            // Check NEC codes
            if (key == 0x0C) leds.led1_state = true;  // GPIO16
            if (key == 0x18) leds.led2_state = true;  // GPIO17
            if (key == 0x5E) leds.led3_state = true;  // GPIO18
            
            // Apply states
            gpio_put(leds.led1_pin, leds.led1_state);
            gpio_put(leds.led2_pin, leds.led2_state);
            gpio_put(leds.led3_pin, leds.led3_state);
            
            sleep_ms(10);
        } else {
            sleep_ms(1);
        }
    }
}
```

### Step 2: Understand the Program Flow

```
┌─────────────────────────────────────────────────────────────────┐
│  Program Flow                                                   │
│                                                                 │
│  1. Initialize UART (stdio_init_all)                            │
│  2. Create LED struct with pins 16, 17, 18                      │
│  3. Initialize GPIO pins as outputs                             │
│  4. Initialize IR receiver on GPIO 5                            │
│  5. Enter infinite loop:                                        │
│     a. Check for IR key press                                   │
│     b. If key received:                                         │
│        - Print the NEC command code                             │
│        - Turn all LEDs off                                      │
│        - Check which button: 0x0C, 0x18, or 0x5E                │
│        - Turn on the matching LED                               │
│        - Apply states to GPIO pins                              │
│     c. Sleep briefly and repeat                                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Step 3: Flash the Binary to Your Pico 2

1. Hold the BOOTSEL button on your Pico 2
2. Plug in the USB cable (while holding BOOTSEL)
3. Release BOOTSEL - a drive called "RPI-RP2" appears
4. Drag and drop `0x0023_structures.uf2` onto the drive
5. The Pico will reboot and start running!

### Step 4: Verify It's Working

**Open PuTTY (115200 baud) and test:**
- Press "1" on remote → Red LED lights, terminal shows `NEC command: 0x0C`
- Press "2" on remote → Green LED lights, terminal shows `NEC command: 0x18`
- Press "3" on remote → Yellow LED lights, terminal shows `NEC command: 0x5E`

---

## 🔬 Part 10: Debugging with GDB (Structures)

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
arm-none-eabi-gdb build\0x0023_structures.elf
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

Disassemble around main to see struct initialization:

```gdb
disassemble 0x10000234,+200
```

Look for the struct member initialization sequence (mov instructions with values 16, 17, 18).

### Step 10: Set a Breakpoint at Main

```gdb
break *0x10000234
```

Reset and run to hit the breakpoint:

```gdb
monitor reset halt
continue
```

### Step 11: Examine the Struct on the Stack

After stepping into main, the struct is initialized on the stack. Examine it:

```gdb
stepi 20
x/6xb $sp
```

You should see the struct layout: `10 11 12 00 00 00` (pins 16, 17, 18 and three false states).

### Step 12: Watch GPIO Initialization

Set a breakpoint on gpio_init and watch each LED pin get initialized:

```gdb
break *0x10000260
continue
info registers r0
```

You should see `r0 = 0x10` (16), `0x11` (17), `0x12` (18) for each call.

### Step 13: Examine IR Key Processing

Set a breakpoint after ir_getkey returns:

```gdb
break *0x10000290
continue
```

Press a button on the remote, then check:

```gdb
info registers r0
```

You'll see the NEC code (0x0C, 0x18, or 0x5E).

### Step 14: Watch the Conditional Checks

Step through the NEC code comparisons:

```gdb
stepi 10
info registers
```

Watch for `cmp r0, #0x0c`, `cmp r0, #0x18`, `cmp r0, #0x5e` instructions.

### Step 15: Examine gpio_put Arguments

Before each gpio_put call, check the pin and state:

```gdb
break *0x100002a0
continue
info registers r0 r1
```

`r0` = GPIO pin number, `r1` = state (0 or 1).

### Step 16: Exit GDB

When done exploring:

```gdb
quit
```

---

## 🔬 Part 11: Setting Up Ghidra for Structures

### Step 17: Start Ghidra

Open a terminal and type:

```powershell
ghidraRun
```

### Step 18: Create a New Project

1. Click **File** → **New Project**
2. Select **Non-Shared Project**
3. Click **Next**
4. Enter Project Name: `0x0023_structures`
5. Click **Finish**

### Step 19: Import the Binary

1. Navigate to the `0x0023_structures/build/` folder
2. **Drag and drop** the `.bin` file into Ghidra's project window

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

## 🔬 Part 12: Resolving Functions - Structures Project

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

### Step 24: Identify gpio_init from Struct Pattern

Look for three consecutive calls with values 16, 17, 18:

```assembly
movs r0, #0x10      ; 16 = GPIO16 (led1_pin)
bl   FUN_xxxxx      ; gpio_init

movs r0, #0x11      ; 17 = GPIO17 (led2_pin)
bl   FUN_xxxxx      ; gpio_init

movs r0, #0x12      ; 18 = GPIO18 (led3_pin)
bl   FUN_xxxxx      ; gpio_init
```

This pattern reveals the struct members! Update the function signature:
1. Right-click → **Edit Function Signature**
2. Change to: `void gpio_init(uint gpio)`
3. Click **OK**

### Step 25: Resolve ir_init

Look for a function call with GPIO 5:

```assembly
movs r0, #0x5       ; GPIO 5 for IR receiver
bl   FUN_xxxxx      ; ir_init
```

1. Right-click → **Edit Function Signature**
2. Change to: `void ir_init(uint pin)`
3. Click **OK**

### Step 26: Resolve printf

Right after ir_init, look for the "IR receiver on GPIO" string being loaded:

1. Right-click → **Edit Function Signature**
2. Change to: `int printf(char *format, ...)`
3. Check the **Varargs** checkbox
4. Click **OK**

### Step 27: Resolve ir_getkey

Look for a function that returns a value checked against conditions:

```assembly
bl   FUN_xxxxx      ; Call ir_getkey
cmp  r0, #0         ; Check if >= 0
blt  no_key         ; If negative, no key pressed
```

1. Right-click → **Edit Function Signature**
2. Change to: `int ir_getkey(void)`
3. Click **OK**

### Step 28: Resolve sleep_ms

Look for calls with 10 (0x0A) or 1 (0x01):

```assembly
movs r0, #0x0A      ; 10 milliseconds
bl   FUN_xxxxx      ; sleep_ms
```

1. Right-click → **Edit Function Signature**
2. Change to: `void sleep_ms(uint ms)`
3. Click **OK**

---

## 🔬 Part 13: Recognizing Struct Patterns in Assembly

### Step 29: Identify GPIO Set Direction

After each `gpio_init`, look for direction setting:

```assembly
mov.w r4, #0x1          ; direction = output (1 = GPIO_OUT)
mcrr  p0, 0x4, r3, r4   ; Configure GPIO direction register
```

This is the compiler's version of `gpio_set_dir(pin, GPIO_OUT)`.

### Step 30: Map the Struct Members

Create a mental (or written) map:

```
┌─────────────────────────────────────────────────────────────────┐
│  Struct Member Mapping                                          │
│                                                                 │
│  Assembly Value   →   Struct Member   →   Physical LED          │
│  ─────────────────────────────────────────────────────────────  │
│  0x10 (16)        →   led1_pin        →   Red LED               │
│  0x11 (17)        →   led2_pin        →   Green LED             │
│  0x12 (18)        →   led3_pin        →   Yellow LED            │
│                                                                 │
│  NEC Code         →   State Member    →   Action                │
│  ─────────────────────────────────────────────────────────────  │
│  0x0C             →   led1_state=true →   Red LED ON            │
│  0x18             →   led2_state=true →   Green LED ON          │
│  0x5E             →   led3_state=true →   Yellow LED ON         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔬 Part 14: Hacking Structures

### Step 31: Open the Bytes Editor

1. Click **Window** → **Bytes**
2. Click the pencil icon to enable editing

### Step 32: Swap LED Pin Assignments

We'll swap the red and green LED pins to reverse their behavior!

**Find the gpio_init calls:**

1. Locate where `0x10` (16) is loaded for led1_pin
2. Change `0x10` to `0x11` (swap red to green's pin)
3. Locate where `0x11` (17) is loaded for led2_pin
4. Change `0x11` to `0x10` (swap green to red's pin)

**Before:**
```
LED 1 (0x0C) → GPIO 16 → Red LED
LED 2 (0x18) → GPIO 17 → Green LED
```

**After:**
```
LED 1 (0x0C) → GPIO 17 → Green LED (SWAPPED!)
LED 2 (0x18) → GPIO 16 → Red LED (SWAPPED!)
```

### Step 33: Export and Flash

1. Click **File** → **Export Program**
2. Set **Format** to **Binary**
3. Name: `0x0023_structures-h.bin`
4. Click **OK**

Convert and flash:

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0023_structures
python ..\uf2conv.py build\0x0023_structures-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

### Step 34: Verify the Hack

**Open PuTTY and test:**
- Press "1" on remote → **GREEN** LED lights (was red!)
- Terminal still shows `NEC command: 0x0C`
- Press "2" on remote → **RED** LED lights (was green!)
- Terminal still shows `NEC command: 0x18`

**The log says one thing, but the hardware does another!**

---

## 🔬 Part 15: Security Implications - Log Desynchronization

### The Danger of Mismatched Logs

```
┌─────────────────────────────────────────────────────────────────┐
│  Log vs Reality Desynchronization                               │
│                                                                 │
│  ┌─────────────────┐          ┌─────────────────┐               │
│  │  Terminal Log   │          │  Physical LEDs  │               │
│  ├─────────────────┤          ├─────────────────┤               │
│  │ NEC: 0x0C       │ ◄─────── │ GREEN LED on    │ ◄── Mismatch! │
│  │ (expects RED)   │          │ (not red!)      │               │
│  ├─────────────────┤          ├─────────────────┤               │
│  │ NEC: 0x18       │ ◄─────── │ RED LED on      │ ◄── Mismatch! │
│  │ (expects GREEN) │          │ (not green!)    │               │
│  └─────────────────┘          └─────────────────┘               │
│                                                                 │
│  The OPERATOR sees correct logs but WRONG physical behavior!   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Real-World Example: Stuxnet

**Stuxnet** was a cyberweapon that:
- Attacked Iranian nuclear centrifuges
- Made centrifuges spin at dangerous speeds
- Fed FALSE "everything normal" data to operators
- Operators saw stable readings while equipment was destroyed

Our LED example demonstrates the same principle:
- Logs show expected behavior
- Hardware performs different actions
- Attackers can hide malicious activity

---

## 🔬 Part 16: Functions Project - Advanced Code

### Step 35: Review the Functions Code

**File: `0x0026_functions.c`** (key functions shown)

```c
// Map IR command to LED number
int ir_to_led_number(int ir_command) {
    if (ir_command == 0x0C) return 1;
    if (ir_command == 0x18) return 2;
    if (ir_command == 0x5E) return 3;
    return 0;
}

// Get GPIO pin for LED number
uint8_t get_led_pin(simple_led_ctrl_t *leds, int led_num) {
    if (led_num == 1) return leds->led1_pin;
    if (led_num == 2) return leds->led2_pin;
    if (led_num == 3) return leds->led3_pin;
    return 0;
}

// Turn off all LEDs
void leds_all_off(simple_led_ctrl_t *leds) {
    gpio_put(leds->led1_pin, false);
    gpio_put(leds->led2_pin, false);
    gpio_put(leds->led3_pin, false);
}

// Blink an LED
void blink_led(uint8_t pin, uint8_t count, uint32_t delay_ms) {
    for (uint8_t i = 0; i < count; i++) {
        gpio_put(pin, true);
        sleep_ms(delay_ms);
        gpio_put(pin, false);
        sleep_ms(delay_ms);
    }
}

// Main command processor
int process_ir_led_command(int ir_command, simple_led_ctrl_t *leds, uint8_t blink_count) {
    if (!leds || ir_command < 0) return -1;
    
    leds_all_off(leds);
    int led_num = ir_to_led_number(ir_command);
    if (led_num == 0) return 0;
    
    uint8_t pin = get_led_pin(leds, led_num);
    blink_led(pin, blink_count, 50);
    gpio_put(pin, true);
    
    return led_num;
}
```

### Step 36: Understand the Function Call Chain

```
┌─────────────────────────────────────────────────────────────────┐
│  Function Call Chain                                            │
│                                                                 │
│  main()                                                         │
│    │                                                            │
│    └──► process_ir_led_command(key, &leds, 3)                   │
│           │                                                     │
│           ├──► leds_all_off(&leds)                              │
│           │      └──► gpio_put() × 3                            │
│           │                                                     │
│           ├──► ir_to_led_number(ir_command)                     │
│           │      └──► returns 1, 2, or 3                        │
│           │                                                     │
│           ├──► get_led_pin(&leds, led_num)                      │
│           │      └──► returns GPIO pin number                   │
│           │                                                     │
│           ├──► blink_led(pin, 3, 50)                            │
│           │      └──► gpio_put() + sleep_ms() in loop           │
│           │                                                     │
│           └──► gpio_put(pin, true)                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Step 37: Flash and Test

1. Flash `0x0026_functions.uf2` to your Pico 2
2. Open PuTTY
3. Press remote buttons:
   - "1" → Red LED blinks 3 times, then stays on
   - "2" → Green LED blinks 3 times, then stays on
   - "3" → Yellow LED blinks 3 times, then stays on

---

## 🔬 Part 17: Debugging with GDB (Functions)

### Step 38: Start OpenOCD (Terminal 1)

Open a terminal and start OpenOCD:

```powershell
openocd ^
  -s "C:\Users\flare-vm\.pico-sdk\openocd\0.12.0+dev\scripts" ^
  -f interface/cmsis-dap.cfg ^
  -f target/rp2350.cfg ^
  -c "adapter speed 5000"
```

You should see output indicating OpenOCD connected successfully to your Pico 2 via the Debug Probe.

### Step 39: Start GDB (Terminal 2)

Open a **new terminal** and launch GDB with the binary:

```powershell
arm-none-eabi-gdb build\0x0026_functions.elf
```

### Step 40: Connect to the Remote Target

In GDB, connect to OpenOCD:

```gdb
target remote :3333
```

### Step 41: Halt the Running Binary

Stop the processor:

```gdb
monitor halt
```

### Step 42: Examine the Function Layout

Disassemble to see the multiple functions:

```gdb
disassemble 0x10000234,+300
```

You'll see multiple function prologues (push) and epilogues (pop) for the helper functions.

### Step 43: Set Breakpoints on Key Functions

Set breakpoints on the helper functions:

```gdb
break *0x10000234
break *0x10000280
break *0x100002a0
```

Reset and run:

```gdb
monitor reset halt
continue
```

### Step 44: Trace the Function Call Chain

When you press a remote button, step through the calls:

```gdb
stepi 50
info registers
```

Watch the call chain: `process_ir_led_command` → `leds_all_off` → `ir_to_led_number` → `get_led_pin` → `blink_led`.

### Step 45: Examine ir_to_led_number

When the comparison function runs, check the return value:

```gdb
info registers r0
```

For button "1", you should see `r0 = 1`. For button "2", `r0 = 2`.

### Step 46: Watch the blink_led Loop

Set a breakpoint inside blink_led and watch it execute 3 times:

```gdb
break *0x100002c0
continue
info registers r0 r1
```

`r0` = pin number, `r1` = state (alternates 0 and 1).

### Step 47: Examine Pointer Dereference

Watch how the struct pointer is used to get LED pins:

```gdb
x/6xb $r0
```

This shows the struct contents when `leds` pointer is in r0.

### Step 48: Watch Return Values

After function calls, check return values in r0:

```gdb
stepi 20
info registers r0
```

### Step 49: Exit GDB

When done exploring:

```gdb
quit
```

---

## 🔬 Part 18: Analyzing .ELF Files in Ghidra

### Step 50: Create New Ghidra Project

1. Create project: `0x0026_functions`
2. Import the `.elf` file (NOT the .bin this time!)

### Why Use .ELF Instead of .BIN?

| Feature        | .BIN File            | .ELF File                   |
| -------------- | -------------------- | --------------------------- |
| **Symbols**    | None                 | Function/variable names     |
| **Sections**   | Raw bytes only       | .text, .data, .rodata, etc. |
| **Debug info** | None                 | May include debug symbols   |
| **Size**       | Smaller              | Larger                      |
| **Use case**   | Flashing to hardware | Analysis and debugging      |

### Step 51: Import and Analyze the .ELF

1. Drag and drop the `.elf` file into Ghidra
2. Ghidra automatically detects ARM format!
3. Click **Yes** to analyze
4. Wait for analysis to complete

### Step 52: Explore the Symbol Tree

With .ELF files, you get more information:
1. Look at the **Symbol Tree** panel
2. Expand **Functions** - you may see named functions!
3. Expand **Labels** - data labels may appear

---

## 🔬 Part 19: Hacking the Functions Project

### Step 53: Find LED Pin Values

Look for the struct initialization pattern:

```assembly
movs r0, #0x10      ; led1_pin = 16
movs r0, #0x11      ; led2_pin = 17
movs r0, #0x12      ; led3_pin = 18
```

### Step 54: Swap LED 1 and LED 3

We'll swap the red (GPIO 16) and yellow (GPIO 18) LEDs:

**Find and patch in the .bin file:**
1. Change `0x10` (16) to `0x12` (18)
2. Change `0x12` (18) to `0x10` (16)

**Before:**
```
Button 1 → LED 1 → GPIO 16 → Red
Button 3 → LED 3 → GPIO 18 → Yellow
```

**After:**
```
Button 1 → LED 1 → GPIO 18 → Yellow (SWAPPED!)
Button 3 → LED 3 → GPIO 16 → Red (SWAPPED!)
```

### Step 55: Export the Patched .BIN

**Important:** Even though we analyzed the .elf, we patch the .bin!

1. Open the original `.bin` file in Ghidra (or a hex editor)
2. Apply the patches
3. Export as `0x0026_functions-h.bin`

### Step 56: Convert and Flash

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0026_functions
python ..\uf2conv.py build\0x0026_functions-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

### Step 57: Verify the Hack

**Open PuTTY and test:**
- Press "1" → **YELLOW** LED blinks (was red!)
- Terminal shows: `LED 1 activated on GPIO 16` (WRONG - it's actually GPIO 18!)
- Press "3" → **RED** LED blinks (was yellow!)
- Terminal shows: `LED 3 activated on GPIO 18` (WRONG - it's actually GPIO 16!)

**Again, logs don't match reality!**

---

## 📊 Part 20: Summary and Review

### What We Accomplished

1. **Learned C structures** - Grouping related data together
2. **Understood struct memory layout** - How members are stored consecutively
3. **Mastered dot and arrow operators** - Accessing struct members
4. **Learned the NEC IR protocol** - How remotes communicate
5. **Understood functions with parameters** - Passing data in and out
6. **Saw struct flattening in assembly** - How compilers transform structs
7. **Analyzed .ELF files** - Getting more symbol information
8. **Hacked GPIO assignments** - Swapping LED behavior
9. **Discovered log desynchronization** - Security implications

### Struct Operations Summary

```
┌─────────────────────────────────────────────────────────────────┐
│  Struct Operations                                              │
│                                                                 │
│  Definition:                                                    │
│  typedef struct {                                               │
│      uint8_t pin;                                               │
│      bool state;                                                │
│  } led_t;                                                       │
│                                                                 │
│  Creation:                                                      │
│  led_t led = { .pin = 16, .state = false };                     │
│                                                                 │
│  Access (variable):     led.pin                                 │
│  Access (pointer):      ptr->pin  or  (*ptr).pin                │
│                                                                 │
│  Passing to function:   void func(led_t *led)                   │
│  Calling:               func(&led)                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Function Types Summary

```
┌─────────────────────────────────────────────────────────────────┐
│  Function Patterns                                              │
│                                                                 │
│  No params, no return:                                          │
│  void leds_all_off(void)                                        │
│                                                                 │
│  With params, no return:                                        │
│  void blink_led(uint8_t pin, uint8_t count, uint32_t delay)     │
│                                                                 │
│  No params, with return:                                        │
│  int ir_getkey(void)                                            │
│                                                                 │
│  With params, with return:                                      │
│  int ir_to_led_number(int ir_command)                           │
│                                                                 │
│  With struct pointer:                                           │
│  uint8_t get_led_pin(simple_led_ctrl_t *leds, int led_num)      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Key Memory Addresses

| Memory Address | Description                     |
| -------------- | ------------------------------- |
| `0x10000234`   | main() function                 |
| `0x10` (16)    | GPIO 16 - Red LED (led1_pin)    |
| `0x11` (17)    | GPIO 17 - Green LED (led2_pin)  |
| `0x12` (18)    | GPIO 18 - Yellow LED (led3_pin) |
| `0x05`         | GPIO 5 - IR receiver            |
| `0x0C`         | NEC code for button 1           |
| `0x18`         | NEC code for button 2           |
| `0x5E`         | NEC code for button 3           |

---

## ✅ Practice Exercises

### Exercise 1: Add a Fourth LED
Modify the struct to include a fourth LED on GPIO 19.

**Hint:** Add `led4_pin` and `led4_state` members.

### Exercise 2: Change Blink Count
Find and modify the blink count from 3 to 5 blinks.

**Hint:** Look for the value passed to `process_ir_led_command`.

### Exercise 3: Swap All Three LEDs
Create a rotation where 1→Green, 2→Yellow, 3→Red.

**Hint:** Patch all three GPIO values.

### Exercise 4: Change Blink Speed
Make the LEDs blink faster by changing the delay from 50ms to 25ms.

**Hint:** Find `0x32` (50) in the function parameters.

### Exercise 5: Disable One LED
Make button 2 do nothing (LED stays off).

**Hint:** NOP out the gpio_put call or change the NEC code comparison.

---

## 🎓 Key Takeaways

1. **Structs group related data** - Better organization than separate variables

2. **Dot operator for variables, arrow for pointers** - `.` vs `->`

3. **Designated initializers are cleaner** - `.member = value` syntax

4. **Compilers flatten structs** - You see values, not struct names, in assembly

5. **NEC protocol uses 8-bit commands** - 0x0C, 0x18, 0x5E for our buttons

6. **Functions separate concerns** - Each function does one job

7. **.ELF files contain more info than .BIN** - Symbols, sections, debug data

8. **Log desynchronization is dangerous** - Logs can lie about real behavior

9. **Pattern recognition is key** - Consecutive values like 16, 17, 18 reveal structs

10. **Always patch the .bin for flashing** - .elf is for analysis only

---

## 📖 Glossary

| Term                       | Definition                                         |
| -------------------------- | -------------------------------------------------- |
| **Arrow Operator (->)**    | Accesses struct member through a pointer           |
| **Designated Initializer** | Syntax `.member = value` for struct initialization |
| **Dot Operator (.)**       | Accesses struct member from a struct variable      |
| **.ELF File**              | Executable and Linkable Format - contains symbols  |
| **Flattening**             | Compiler converting structs to individual values   |
| **IR (Infrared)**          | Invisible light used for remote control            |
| **Log Desynchronization**  | When logs don't match actual system behavior       |
| **Member**                 | A variable inside a struct                         |
| **NEC Protocol**           | Common IR communication standard                   |
| **Struct**                 | User-defined type grouping related variables       |
| **typedef**                | Creates an alias for a type                        |

---

## 🔗 Additional Resources

### NEC IR Command Reference

| Button | Command | Binary    |
| ------ | ------- | --------- |
| 1      | 0x0C    | 0000 1100 |
| 2      | 0x18    | 0001 1000 |
| 3      | 0x5E    | 0101 1110 |

### GPIO Pin Quick Reference

| GPIO | Default Function | Our Usage   |
| ---- | ---------------- | ----------- |
| 5    | General I/O      | IR Receiver |
| 16   | General I/O      | Red LED     |
| 17   | General I/O      | Green LED   |
| 18   | General I/O      | Yellow LED  |

### Struct Size Calculation

| Type       | Size (bytes) |
| ---------- | ------------ |
| `uint8_t`  | 1            |
| `bool`     | 1            |
| `uint16_t` | 2            |
| `uint32_t` | 4            |
| `int`      | 4            |
| `float`    | 4            |
| `pointer`  | 4 (on ARM32) |

---

## 🚨 Real-World Implications

### What You've Learned in This Course

Over these weeks, you've built skills that few people possess:

1. **Hardware fundamentals** - GPIO, I2C, PWM, IR protocols
2. **Reverse engineering** - Ghidra, disassembly, function identification
3. **Binary patching** - Modifying compiled code
4. **Security awareness** - Understanding vulnerabilities

### The Power and Responsibility

The techniques you've learned can be used for:

**Good:**
- Security research
- Debugging proprietary systems
- Understanding how things work
- Career in cybersecurity

**Danger:**
- Unauthorized system access
- Sabotage of critical infrastructure
- Fraud and deception

**Always use your skills ethically and legally!**

### Keep Learning

This is just the beginning:
- Explore more complex protocols (SPI, CAN bus)
- Learn dynamic analysis with debuggers
- Study cryptographic implementations
- Practice on CTF challenges

---

**Congratulations on completing this course! You now have the curiosity, persistence, and skills that embedded systems engineers and security researchers thrive on. Keep experimenting, documenting, and sharing your work. The world needs more builders and defenders like you!**

Happy hacking! 🔧

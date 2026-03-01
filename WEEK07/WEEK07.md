# Week 7: Constants in Embedded Systems: Debugging and Hacking Constants w/ 1602 LCD I2C Basics

## 🎯 What You'll Learn This Week

By the end of this tutorial, you will be able to:
- Understand the difference between `#define` macros and `const` variables
- Know how constants are stored differently in memory (compile-time vs runtime)
- Understand the I²C (Inter-Integrated Circuit) communication protocol
- Configure I²C peripherals and communicate with LCD displays
- Understand C structs and how the Pico SDK uses them for hardware abstraction
- Use GDB to examine constants, structs, and string literals in memory
- Hack constant values and string literals using a hex editor
- Patch LCD display text without access to source code

---

## 📚 Part 1: Understanding Constants in C

### Two Types of Constants

In C, there are two ways to create values that shouldn't change:

| Type        | Syntax                  | Where It Lives     | When Resolved |
| ----------- | ----------------------- | ------------------ | ------------- |
| **#define** | `#define FAV_NUM 42`    | Nowhere (replaced) | Compile time  |
| **const**   | `const int NUM = 1337;` | Flash (.rodata)    | Runtime       |

### Preprocessor Macros (#define)

A **preprocessor macro** is a text replacement that happens BEFORE your code is compiled:

```c
#define FAV_NUM 42

printf("Value: %d", FAV_NUM);
// Becomes: printf("Value: %d", 42);
```

Think of it like a "find and replace" in a text editor. The compiler never sees `FAV_NUM` - it only sees `42`!

```
┌─────────────────────────────────────────────────────────────────┐
│  Preprocessor Macro Flow                                        │
│                                                                 │
│  Source Code          Preprocessor         Compiler             │
│  ┌──────────┐        ┌──────────┐        ┌──────────┐           │
│  │ #define  │        │ Replace  │        │ Compile  │           │
│  │ FAV_NUM  │ ─────► │ FAV_NUM  │ ─────► │ binary   │           │
│  │ 42       │        │ with 42  │        │ code     │           │
│  └──────────┘        └──────────┘        └──────────┘           │
│                                                                 │
│  FAV_NUM doesn't exist in the final binary!                     │
│  The value 42 is embedded directly in instructions.             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Const Variables

A **const variable** is an actual variable stored in memory, but marked as read-only:

```c
const int OTHER_FAV_NUM = 1337;
```

Unlike `#define`, this creates a real memory location in the `.rodata` (read-only data) section of flash:

```
┌─────────────────────────────────────────────────────────────────┐
│  Const Variable in Memory                                       │
│                                                                 │
│  Flash Memory (.rodata section)                                 │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ Address: 0x10001234                                        │ │
│  │ Value:   0x00000539 (1337 in hex)                          │ │
│  │ Name:    OTHER_FAV_NUM (in debug symbols only)             │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                 │
│  The variable EXISTS in memory and can be read at runtime.      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Comparison: #define vs const

| Feature              | #define                  | const                        |
| -------------------- | ------------------------ | ---------------------------- |
| **Type checking**    | None (just text)         | Yes (compiler enforced)      |
| **Memory usage**     | None (inlined)           | Uses flash space             |
| **Debugger visible** | No                       | Yes (with symbols)           |
| **Can take address** | No (`&FAV_NUM` fails)    | Yes (`&OTHER_FAV_NUM` works) |
| **Scope**            | Global (from definition) | Normal C scoping rules       |

---

## 📚 Part 2: Understanding I²C Communication

### What is I²C?

**I²C** (pronounced "I-squared-C" or "I-two-C") stands for **Inter-Integrated Circuit**. It's a way for chips to talk to each other using just TWO wires!

```
┌─────────────────────────────────────────────────────────────────┐
│  I²C Bus - Two Wires, Many Devices                              │
│                                                                 │
│     3.3V                                                        │
│       │                                                         │
│       ┴ Pull-up    ┴ Pull-up                                    │
│       │            │                                            │
│  SDA ─┼────────────┼───────────────────────────────────────     │
│       │            │                                            │
│  SCL ─┼────────────┼───────────────────────────────────────     │
│       │            │            │            │                  │
│   ┌───┴────┐    ┌──┴───┐   ┌────┴──┐   ┌─────┴───┐              │
│   │ Pico   │    │  LCD │   │Sensor │   │ EEPROM  │              │
│   │(Master)│    │ 0x27 │   │ 0x48  │   │ 0x50    │              │
│   └────────┘    └──────┘   └───────┘   └─────────┘              │
│                                                                 │
│  Each device has a unique address (0x27, 0x48, 0x50...)         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The Two I²C Wires

| Wire    | Name         | Purpose                              |
| ------- | ------------ | ------------------------------------ |
| **SDA** | Serial Data  | Carries the actual data bits         |
| **SCL** | Serial Clock | Timing signal that synchronizes data |

### Why Pull-Up Resistors?

I²C uses **open-drain** signals, meaning devices can only pull the line LOW. They can't drive it HIGH! Pull-up resistors are needed to bring the lines back to HIGH when no device is pulling them down.

The Pico 2 has internal pull-ups that we can enable with `gpio_pull_up()`.

### I²C Addresses

Every I²C device has a unique **7-bit address**. Common addresses:

| Device Type           | Typical Address  |
| --------------------- | ---------------- |
| 1602 LCD with PCF8574 | `0x27` or `0x3F` |
| Temperature sensor    | `0x48`           |
| EEPROM                | `0x50`           |
| Real-time clock       | `0x68`           |

### I²C Communication Flow

```
┌─────────────────────────────────────────────────────────────────┐
│  I²C Transaction                                                │
│                                                                 │
│  1. Master sends START condition                                │
│  2. Master sends device address (7 bits) + R/W bit              │
│  3. Addressed device sends ACK (acknowledge)                    │
│  4. Data is transferred (8 bits at a time)                      │
│  5. Receiver sends ACK after each byte                          │
│  6. Master sends STOP condition                                 │
│                                                                 │
│  START ──► Address ──► ACK ──► Data ──► ACK ──► STOP            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📚 Part 3: Understanding C Structs

### What is a Struct?

A **struct** (short for "structure") is a way to group related variables together under one name. Think of it like a form with multiple fields:

```c
// A struct definition - like a template
struct student {
    char name[50];
    int age;
    float gpa;
};

// Creating a variable of this struct type
struct student alice = {"Alice", 16, 3.8};
```

### Why Use Structs?

Instead of passing many separate variables:
```c
void print_student(char *name, int age, float gpa); // Messy!
```

You pass one struct:
```c
void print_student(struct student s); // Clean!
```

### The typedef Keyword

Writing `struct student` everywhere is tedious. The `typedef` keyword creates an alias:

```c
typedef struct student student_t;

// Now you can write:
student_t alice;  // Instead of: struct student alice;
```

### Forward Declaration

Sometimes you need to tell the compiler "this struct exists" before defining it:

```c
typedef struct i2c_inst i2c_inst_t;  // Forward declaration + alias

// Later, the full definition:
struct i2c_inst {
    i2c_hw_t *hw;
    bool restart_on_next;
};
```

---

## 📚 Part 4: Understanding the Pico SDK's I²C Structs

### The i2c_inst_t Struct

The Pico SDK uses a struct to represent each I²C controller:

```c
struct i2c_inst {
    i2c_hw_t *hw;           // Pointer to hardware registers
    bool restart_on_next;   // SDK internal flag
};
```

**What each member means:**

| Member            | Type         | Purpose                                  |
| ----------------- | ------------ | ---------------------------------------- |
| `hw`              | `i2c_hw_t *` | Pointer to the actual hardware registers |
| `restart_on_next` | `bool`       | Tracks if next transfer needs a restart  |

### The Macro Chain

When you write `I2C_PORT` in your code, here's what happens:

```
┌─────────────────────────────────────────────────────────────────┐
│  Macro Expansion Chain                                          │
│                                                                 │
│  In your code:     #define I2C_PORT i2c1                        │
│                           │                                     │
│                           ▼                                     │
│  In i2c.h:         #define i2c1 (&i2c1_inst)                    │
│                           │                                     │
│                           ▼                                     │
│  In i2c.c:         i2c_inst_t i2c1_inst = {i2c1_hw, false};     │
│                           │                                     │
│                           ▼                                     │
│  In i2c.h:         #define i2c1_hw ((i2c_hw_t *)I2C1_BASE)      │
│                           │                                     │
│                           ▼                                     │
│  In addressmap.h:  #define I2C1_BASE 0x40098000                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

So `I2C_PORT` eventually becomes a pointer to a struct that contains a pointer to hardware registers at address `0x40098000`!

### The Hardware Register Pointer

The `i2c_hw_t *hw` member points to the actual silicon:

```
┌─────────────────────────────────────────────────────────────────┐
│  Memory Map                                                     │
│                                                                 │
│  Address 0x40098000: I²C1 Hardware Registers                    │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ Offset 0x00: IC_CON (Control register)                     │ │
│  │ Offset 0x04: IC_TAR (Target address register)              │ │
│  │ Offset 0x10: IC_DATA_CMD (Data command register)           │ │
│  │ ...                                                        │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                 │
│  The i2c_hw_t struct maps directly to these registers!          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📚 Part 5: The ARM Calling Convention (AAPCS)

### How Arguments Are Passed

On ARM Cortex-M, the **ARM Architecture Procedure Call Standard (AAPCS)** defines how functions receive arguments:

| Register | Purpose          |
| -------- | ---------------- |
| `r0`     | First argument   |
| `r1`     | Second argument  |
| `r2`     | Third argument   |
| `r3`     | Fourth argument  |
| Stack    | Fifth+ arguments |
| `r0`     | Return value     |

### Example: i2c_init(i2c1, 100000)

```c
i2c_init(I2C_PORT, 100000);
```

In assembly:
```assembly
ldr r0, [address of i2c1_inst]   ; r0 = pointer to struct (first arg)
ldr r1, =0x186A0                 ; r1 = 100000 (second arg)
bl  i2c_init                     ; Call the function
```

---

## 📚 Part 6: Setting Up Your Environment

### Prerequisites

Before we start, make sure you have:
1. A Raspberry Pi Pico 2 board
2. A Raspberry Pi Pico Debug Probe
3. OpenOCD installed and configured
4. GDB (`arm-none-eabi-gdb`) installed
5. Python installed (for UF2 conversion)
6. A serial monitor (PuTTY, minicom, or screen)
7. A 1602 LCD display with I²C backpack (PCF8574)
8. A hex editor (HxD, ImHex, or similar)
9. The sample project: `0x0017_constants`

### Hardware Setup

Connect your LCD like this:

| LCD Pin | Pico 2 Pin |
| ------- | ---------- |
| VCC     | 3.3V or 5V |
| GND     | GND        |
| SDA     | GPIO 2     |
| SCL     | GPIO 3     |

```
┌─────────────────────────────────────────────────────────────────┐
│  I²C LCD Wiring                                                 │
│                                                                 │
│  Pico 2                        1602 LCD + I²C Backpack          │
│  ┌──────────┐                  ┌──────────────────────┐         │
│  │          │                  │                      │         │
│  │ GPIO 2   │─────── SDA ─────►│ SDA                  │         │
│  │ (SDA)    │                  │                      │         │
│  │          │                  │    ┌────────────┐    │         │
│  │ GPIO 3   │─────── SCL ─────►│ SCL│  Reverse   │    │         │
│  │ (SCL)    │                  │    │Engineering │    │         │
│  │          │                  │    └────────────┘    │         │
│  │ 3.3V     │─────── VCC ─────►│ VCC                  │         │
│  │          │                  │                      │         │
│  │ GND      │─────── GND ─────►│ GND                  │         │
│  │          │                  │                      │         │
│  └──────────┘                  └──────────────────────┘         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Project Structure

```
Embedded-Hacking/
├── 0x0017_constants/
│   ├── build/
│   │   ├── 0x0017_constants.uf2
│   │   └── 0x0017_constants.bin
│   ├── 0x0017_constants.c
│   └── lcd_1602.h
└── uf2conv.py
```

---

## 🔬 Part 7: Hands-On Tutorial - Constants and I²C LCD

### Step 1: Review the Source Code

Let's examine the constants code:

**File: `0x0017_constants.c`**

```c
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_1602.h"

#define FAV_NUM 42
#define I2C_PORT i2c1
#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

const int OTHER_FAV_NUM = 1337;

int main(void) {
    stdio_init_all();
    
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    lcd_i2c_init(I2C_PORT, 0x27, 4, 0x08);
    lcd_set_cursor(0, 0);
    lcd_puts("Reverse");
    lcd_set_cursor(1, 0);
    lcd_puts("Engineering");
    
    while (true) {
        printf("FAV_NUM: %d\r\n", FAV_NUM);
        printf("OTHER_FAV_NUM: %d\r\n", OTHER_FAV_NUM);
    }
}
```

**What this code does:**

1. **Lines 7-10:** Define preprocessor macros for constants and I²C configuration
2. **Line 12:** Define a `const` variable stored in flash
3. **Line 15:** Initialize UART for serial output
4. **Lines 17-21:** Initialize I²C1 at 100kHz, configure GPIO pins, enable pull-ups
5. **Lines 23-27:** Initialize LCD and display "Reverse" on line 0, "Engineering" on line 1
6. **Lines 29-32:** Infinite loop printing both constant values to serial terminal

### Step 2: Flash the Binary to Your Pico 2

1. Hold the BOOTSEL button on your Pico 2
2. Plug in the USB cable (while holding BOOTSEL)
3. Release BOOTSEL - a drive called "RPI-RP2" appears
4. Drag and drop `0x0017_constants.uf2` onto the drive
5. The Pico will reboot and start running!

### Step 3: Verify It's Working

**Check the LCD:**
- Line 1 should show: `Reverse`
- Line 2 should show: `Engineering`

**Check the serial monitor (PuTTY/screen):**
```
FAV_NUM: 42
OTHER_FAV_NUM: 1337
FAV_NUM: 42
OTHER_FAV_NUM: 1337
...
```

---

## 🔬 Part 8: Debugging with GDB (Dynamic Analysis)

> 🔄 **REVIEW:** This setup is identical to previous weeks. If you need a refresher on OpenOCD and GDB connection, refer back to Week 3 Part 6.

### Starting the Debug Session

**Terminal 1 - Start OpenOCD:**

```bash
openocd ^
  -s "C:\Users\flare-vm\.pico-sdk\openocd\0.12.0+dev\scripts" ^
  -f interface/cmsis-dap.cfg ^
  -f target/rp2350.cfg ^
  -c "adapter speed 5000"
```

**Terminal 2 - Start GDB:**

```bash
arm-none-eabi-gdb build/0x0017_constants.elf
```

**Connect to target:**

```gdb
(gdb) target remote :3333
(gdb) monitor reset halt
```

### Step 4: Examine Main Function

Let's examine the main function. Disassemble from the entry point:

```
x/60i 0x10000234
```

You should see output like:

```
0x10000234: push {r4, r5, r6, r7, lr}
0x10000236: sub sp, #12
0x10000238: bl 0x10003014              ; stdio_init_all
0x1000023c: ldr r0, [pc, #108]         ; Load i2c1_inst pointer
0x1000023e: ldr r1, =0x186A0           ; 100000 (baud rate)
0x10000240: bl 0x100002b4              ; i2c_init
0x10000244: movs r0, #2                ; GPIO 2 (SDA)
0x10000246: movs r1, #3                ; GPIO_FUNC_I2C
0x10000248: bl 0x100002c8              ; gpio_set_function
0x1000024c: movs r0, #3                ; GPIO 3 (SCL)
0x1000024e: movs r1, #3                ; GPIO_FUNC_I2C
0x10000250: bl 0x100002c8              ; gpio_set_function
...
```

### Step 5: Set a Breakpoint at Main

```
b *0x10000234
c
```

GDB responds:
```
Breakpoint 1 at 0x10000234
Continuing.

Breakpoint 1, 0x10000234 in ?? ()
```

### Step 6: Find the #define Constant (FAV_NUM)

Step through to the printf call and examine the registers:

```
x/20i 0x1000028e
```

Look for:
```
0x1000028e: movs r1,#0x2a    ; 0x2a = 42 decimal (FAV_NUM)
```

The `#define` constant is embedded directly as an immediate value in the instruction!

### Step 7: Find the const Variable (OTHER_FAV_NUM)

Continue examining the code:

```
x/10i 0x10000296
```

Look for a load instruction:
```
0x10000296: ldr r1, [pc, #offset]    ; Load OTHER_FAV_NUM from memory
```

The `const` variable is loaded from a memory address because it actually exists in flash!

### Step 8: Examine the const Value in Memory

Find where OTHER_FAV_NUM is stored:

```
x/1wx 0x100002ac
```

This should show the address pointing to the value. Then examine that address:

```
x/1wd 0x10003xxx
```

You should see `1337` (or `0x539` in hex).

### Step 9: Examine the I²C Struct

Find the i2c1_inst struct address loaded into r0 before i2c_init:

```
x/2wx 0x2000062c
```

You should see:
```
0x2000062c: 0x40098000    ; hw pointer (I2C1 hardware base)
0x20000630: 0x00000000    ; restart_on_next = false
```

### Step 10: Examine the LCD String Literals

Find the strings passed to lcd_puts:

```
x/s 0x10003ee8
```

Output:
```
0x10003ee8: "Reverse"
```

```
x/s 0x10003ef0
```

Output:
```
0x10003ef0: "Engineering"
```

### Step 11: Step Through I²C Initialization

Use `si` to step through instructions and watch the I²C setup:

```
si
i r r0 r1
```

Observe how:
- `r0` gets the i2c_inst pointer
- `r1` gets the baud rate (100000)
- The function call happens via `bl`

---

## 🔬 Part 9: Understanding the Assembly

Now that we've explored the binary in GDB, let's make sense of the key patterns we found.

### Step 12: Analyze #define vs const in Assembly

From GDB, we discovered a critical difference:

**For FAV_NUM (42) — a `#define` macro:**
```
0x1000028e:    movs r1, #0x2a    ; 0x2a = 42 decimal
```

The value is embedded **directly as an immediate** in the instruction. There is no memory location — the preprocessor replaced `FAV_NUM` with `42` before compilation.

**For OTHER_FAV_NUM (1337) — a `const` variable:**
```
0x10000296:    ldr r1, [pc, #offset]    ; Load value from flash
```

The value is **loaded from a memory address** because `const` creates a real variable stored in the `.rodata` section of flash.

### Step 13: Analyze the I²C Struct Layout

In GDB, we examined the `i2c1_inst` struct at `0x2000062c`:

```gdb
(gdb) x/2wx 0x2000062c
0x2000062c: 0x40098000    0x00000000
```

This maps to the `i2c_inst_t` struct:

```
┌─────────────────────────────────────────────────────────────────┐
│  i2c_inst_t at 0x2000062c                                       │
│                                                                 │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ Offset   Type          Name              Value             │ │
│  │ 0x00     i2c_hw_t *    hw                0x40098000        │ │
│  │ 0x04     bool          restart_on_next   0x00 (false)      │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

The first member (`hw`) points to `0x40098000` — the I²C1 hardware register base. This is the end of the macro chain: `I2C_PORT` → `i2c1` → `&i2c1_inst` → `hw` → `0x40098000`.

### Step 14: Locate the String Literals

We found the LCD strings in flash memory:

```gdb
(gdb) x/s 0x10003ee8
0x10003ee8: "Reverse"

(gdb) x/s 0x10003ef0
0x10003ef0: "Engineering"
```

These are stored consecutively in the `.rodata` section. Note the addresses — we'll need them for patching.

---

## 🔬 Part 10: Hacking the Binary with a Hex Editor

Now for the fun part — we'll patch the `.bin` file directly using a hex editor!

> 💡 **Why a hex editor?** GDB can modify values in RAM at runtime, but those changes are lost when the device reboots. To make **permanent** changes, we edit the `.bin` file on disk and re-flash it.

### Step 15: Open the Binary in a Hex Editor

1. Open **HxD** (or your preferred hex editor: ImHex, 010 Editor, etc.)
2. Click **File** → **Open**
3. Navigate to `0x0017_constants/build/`
4. Open `0x0017_constants.bin`

### Step 16: Calculate the File Offset

The binary is loaded at base address `0x10000000`. To find the file offset of any address:

```
file_offset = address - 0x10000000
```

For example:
- Address `0x1000028e` → file offset `0x28E` (654 in decimal)
- Address `0x10003ee8` → file offset `0x3EE8` (16104 in decimal)

### Step 17: Hack #1 — Change FAV_NUM from 42 to 43

From GDB, we know the instruction at `0x1000028e` is:

```
movs r1, #0x2a    →    bytes: 2a 21
```

To change the value from 42 (`0x2a`) to 43 (`0x2b`):

1. In HxD, press **Ctrl+G** (Go to offset)
2. Enter offset: `28E`
3. You should see the byte `2A` at this position
4. Change `2A` to `2B`
5. The instruction is now `movs r1, #0x2b` (43 in decimal)

> 🔍 **How Thumb encoding works:** In `movs r1, #imm8`, the immediate value is the first byte, and the opcode `21` is the second byte. So the bytes `2a 21` encode `movs r1, #0x2a`.

### Step 18: Hack #2 — Change OTHER_FAV_NUM from 1337 to 1344

The `const` value 1337 (`0x539`) is stored in the data section, not as an instruction immediate. From GDB, we found its memory address.

Examine the literal pool to find where `0x539` is stored:

```gdb
(gdb) x/4wx 0x100002a8
```

Look for the word `0x00000539`. Calculate its file offset and patch it:

1. In HxD, go to the offset where `0x539` is stored
2. The bytes will be `39 05 00 00` (little-endian)
3. Change to `40 05 00 00` (`0x540` = 1344 in decimal)

> 🔍 **Little-endian byte order:** The RP2350 stores multi-byte values with the least significant byte first. So `0x00000539` appears as `39 05 00 00` in memory.

### Step 19: Hack #3 — Change LCD Text from "Reverse" to "Exploit"

**IMPORTANT:** The new string must be the **same length** as the original! "Reverse" and "Exploit" are both 7 characters — perfect!

From GDB, we know "Reverse" is at address `0x10003ee8`:

1. In HxD, press **Ctrl+G** and enter offset: `3EE8`
2. You should see the bytes for "Reverse": `52 65 76 65 72 73 65 00`
3. Change the bytes to spell "Exploit": `45 78 70 6c 6f 69 74 00`

**ASCII Reference:**

| Character | Hex    |
| --------- | ------ |
| E         | `0x45` |
| x         | `0x78` |
| p         | `0x70` |
| l         | `0x6c` |
| o         | `0x6f` |
| i         | `0x69` |
| t         | `0x74` |

### Step 20: Save the Patched Binary

1. Click **File** → **Save As**
2. Save as `0x0017_constants-h.bin` in the build directory
3. Close the hex editor

---

## 🔬 Part 11: Converting and Flashing the Hacked Binary

### Step 21: Convert to UF2 Format

Open a terminal and navigate to your project directory:

```bash
cd Embedded-Hacking/0x0017_constants
```

Run the conversion command:

```bash
python ../uf2conv.py build/0x0017_constants-h.bin --base 0x10000000 --family 0xe48bff59 --output build/hacked.uf2
```

### Step 22: Flash the Hacked Binary

1. Hold BOOTSEL and plug in your Pico 2
2. Drag and drop `hacked.uf2` onto the RPI-RP2 drive
3. Check your LCD and serial monitor

### Step 23: Verify the Hacks

**Check the LCD:**
- Line 1 should now show: `Exploit` (instead of "Reverse")
- Line 2 should still show: `Engineering`

**Check the serial monitor:**
```
FAV_NUM: 43
OTHER_FAV_NUM: 1344
FAV_NUM: 43
OTHER_FAV_NUM: 1344
...
```

🎉 **BOOM! We successfully:**
1. Changed FAV_NUM from 42 to 43
2. Changed OTHER_FAV_NUM from 1337 to 1344
3. Changed the LCD text from "Reverse" to "Exploit"

---

## 📊 Part 12: Summary and Review

### What We Accomplished

1. **Learned about constants** - `#define` macros vs `const` variables
2. **Understood I²C communication** - Two-wire protocol for peripheral communication
3. **Explored C structs** - How the Pico SDK abstracts hardware
4. **Mastered the macro chain** - From `I2C_PORT` to `0x40098000`
5. **Examined structs in GDB** - Inspected memory layout of `i2c_inst_t`
6. **Hacked constant values** - Both immediate and memory-stored using a hex editor
7. **Patched string literals** - Changed LCD display text

### #define vs const Summary

```
┌─────────────────────────────────────────────────────────────────┐
│  #define FAV_NUM 42                                             │
│  ───────────────────                                            │
│  • Text replacement at compile time                             │
│  • No memory allocated                                          │
│  • Cannot take address (&FAV_NUM is invalid)                    │
│  • In binary: value appears as immediate (movs r1, #0x2a)       │
│  • To hack: patch the instruction operand                       │
├─────────────────────────────────────────────────────────────────┤
│  const int OTHER_FAV_NUM = 1337                                 │
│  ──────────────────────────────                                 │
│  • Real variable in .rodata section                             │
│  • Takes memory (4 bytes for int)                               │
│  • Can take address (&OTHER_FAV_NUM is valid)                   │
│  • In binary: value loaded from memory (ldr r1, [address])      │
│  • To hack: patch the value in the data section                 │
└─────────────────────────────────────────────────────────────────┘
```

### I²C Configuration Summary

```
┌─────────────────────────────────────────────────────────────────┐
│  I²C Setup Steps                                                │
│                                                                 │
│  1. i2c_init(i2c1, 100000)        - Initialize at 100kHz        │
│  2. gpio_set_function(pin, I2C)   - Assign pins to I²C          │
│  3. gpio_pull_up(sda_pin)         - Enable SDA pull-up          │
│  4. gpio_pull_up(scl_pin)         - Enable SCL pull-up          │
│  5. lcd_i2c_init(...)             - Initialize the device       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The Struct Chain

```
┌─────────────────────────────────────────────────────────────────┐
│  I2C_PORT → i2c1 → &i2c1_inst → i2c_inst_t                      │
│                                     │                           │
│                                     ├── hw → i2c_hw_t *         │
│                                     │        └── 0x40098000     │
│                                     │                           │
│                                     └── restart_on_next (bool)  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Key Memory Addresses

| Address      | Description                        |
| ------------ | ---------------------------------- |
| `0x10000234` | main() entry point                 |
| `0x1000028e` | FAV_NUM value in instruction       |
| `0x10000296` | OTHER_FAV_NUM load instruction     |
| `0x10003ee8` | "Reverse" string literal (example) |
| `0x40098000` | I²C1 hardware registers base       |
| `0x2000062C` | i2c1_inst struct in SRAM           |

---

## ✅ Practice Exercises

### Exercise 1: Change Both LCD Lines
Change "Engineering" to "Hacking!!!" (same number of characters).

**Hint:** Find the second string after "Reverse" in memory.

### Exercise 2: Change the I²C Address
The LCD is at address `0x27`. Find where this is passed to `lcd_i2c_init` and change it.

**Warning:** If you change to an invalid address, the LCD won't work!

### Exercise 3: Find All String Literals
Search the binary for all readable strings. How many can you find? What do they reveal about the program?

**Hint:** In GDB, use `x/s` to search for strings in the binary, or scan through the `.bin` file in your hex editor.

### Exercise 4: Trace the Struct Pointer
Follow the `i2c1_inst` pointer from the code to SRAM. What values are stored in the struct?

**Hint:** The first member should point to `0x40098000`.

### Exercise 5: Add Your Own Message
Can you make the LCD display your name? Remember the character limit!

**Hint:** Line 1 and Line 2 each have 16 characters maximum on a 1602 LCD.

---

## 🎓 Key Takeaways

1. **#define is text replacement** - It happens before compilation, no memory used.

2. **const creates real variables** - Stored in .rodata, takes memory, has an address.

3. **I²C uses two wires** - SDA for data, SCL for clock, pull-ups required.

4. **Structs group related data** - The SDK uses them to abstract hardware.

5. **Macros can chain** - `I2C_PORT` → `i2c1` → `&i2c1_inst` → hardware pointer.

6. **ARM passes args in registers** - r0-r3 for first four arguments.

7. **GDB reveals struct layouts** - Examine memory to understand data organization.

8. **String hacking requires same length** - Or you'll corrupt adjacent data!

9. **Constants aren't constant** - With binary patching, everything can change!

10. **Compiler optimization changes code** - `gpio_pull_up` becomes `gpio_set_pulls`.

---

## 📖 Glossary

| Term                    | Definition                                          |
| ----------------------- | --------------------------------------------------- |
| **#define**             | Preprocessor directive for text replacement         |
| **AAPCS**               | ARM Architecture Procedure Call Standard            |
| **const**               | Keyword marking a variable as read-only             |
| **Forward Declaration** | Telling compiler a type exists before defining it   |
| **I²C**                 | Inter-Integrated Circuit - two-wire serial protocol |
| **Immediate Value**     | A constant embedded directly in an instruction      |
| **Open-Drain**          | Output that can only pull low, not drive high       |
| **PCF8574**             | Common I²C I/O expander chip used in LCD backpacks  |
| **Preprocessor**        | Tool that processes code before compilation         |
| **Pull-Up Resistor**    | Resistor that holds a line HIGH by default          |
| **SCL**                 | Serial Clock - I²C timing signal                    |
| **SDA**                 | Serial Data - I²C data line                         |
| **Struct**              | User-defined type grouping related variables        |
| **typedef**             | Creates an alias for a type                         |

---

## 🔗 Additional Resources

### I²C Timing Reference

| Speed Mode | Maximum Frequency |
| ---------- | ----------------- |
| Standard   | 100 kHz           |
| Fast       | 400 kHz           |
| Fast Plus  | 1 MHz             |

### Common I²C Addresses

| Device                | Address       |
| --------------------- | ------------- |
| PCF8574 LCD (default) | `0x27`        |
| PCF8574A LCD          | `0x3F`        |
| DS3231 RTC            | `0x68`        |
| BMP280 Sensor         | `0x76`/`0x77` |
| SSD1306 OLED          | `0x3C`/`0x3D` |

### Key ARM Instructions for Constants

| Instruction          | Description                                 |
| -------------------- | ------------------------------------------- |
| `movs rN, #imm`      | Load small immediate (0-255) directly       |
| `ldr rN, [pc, #off]` | Load larger value from literal pool         |
| `ldr rN, =value`     | Pseudo-instruction for loading any constant |

### RP2350 I²C Memory Map

| Address      | Description             |
| ------------ | ----------------------- |
| `0x40090000` | I²C0 hardware registers |
| `0x40098000` | I²C1 hardware registers |

---

**Remember:** When you see complex nested structures in a binary, take your time to understand the hierarchy. Use GDB to examine struct layouts in memory and trace pointer chains. And always remember — even "constants" can be hacked!

Happy hacking! 🔧

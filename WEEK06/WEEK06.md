# Week 6: Static Variables in Embedded Systems: Debugging and Hacking Static Variables w/ GPIO Input Basics

## 🎯 What You'll Learn This Week

By the end of this tutorial, you will be able to:
- Understand the difference between regular (automatic) variables and static variables
- Know where different types of variables are stored (stack vs static storage)
- Configure GPIO pins as inputs and use internal pull-up resistors
- Read button states using `gpio_get()` and control LEDs based on input
- Use GDB to examine how the compiler handles static vs automatic variables
- Identify compiler optimizations by stepping through assembly
- Hack variable values and invert GPIO input/output logic using a hex editor
- Convert patched binaries to UF2 format for flashing

---

## 📚 Part 1: Understanding Static Variables

### What is a Static Variable?

A **static variable** is a special kind of variable that "remembers" its value between function calls or loop iterations. Unlike regular variables that get created and destroyed each time, static variables **persist** for the entire lifetime of your program.

Think of it like this:
- **Regular variable:** Like writing on a whiteboard that gets erased after each class
- **Static variable:** Like writing in a notebook that you keep forever

```
┌─────────────────────────────────────────────────────────────────┐
│  Regular vs Static Variables                                    │
│                                                                 │
│  REGULAR (automatic):                                           │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ Loop 1: Create → Set to 42 → Increment to 43 → Destroy     │ │
│  │ Loop 2: Create → Set to 42 → Increment to 43 → Destroy     │ │
│  │ Loop 3: Create → Set to 42 → Increment to 43 → Destroy     │ │
│  │ Result: Always appears as 42!                              │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                 │
│  STATIC:                                                        │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ Loop 1: Already exists → Read 42 → Increment → Store 43    │ │
│  │ Loop 2: Already exists → Read 43 → Increment → Store 44    │ │
│  │ Loop 3: Already exists → Read 44 → Increment → Store 45    │ │
│  │ Result: Keeps incrementing!                                │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The `static` Keyword

In C, you declare a static variable by adding the `static` keyword:

```c
uint8_t regular_fav_num = 42;       // Regular - recreated each time
static uint8_t static_fav_num = 42; // Static - persists forever
```

### Where Do Variables Live in Memory?

Different types of variables are stored in different memory locations:

| Variable Type     | Storage Location | Lifetime                  | Example                              |
| ----------------- | ---------------- | ------------------------- | ------------------------------------ |
| Automatic (local) | Stack            | Until function/block ends | `uint8_t x = 5;`                     |
| Static            | Static Storage   | Entire program lifetime   | `static uint8_t x = 5;`              |
| Global            | Static Storage   | Entire program lifetime   | `uint8_t x = 5;` (outside functions) |
| Dynamic (heap)    | Heap             | Until `free()` is called  | `malloc(sizeof(int))`                |

### Stack vs Static Storage vs Heap

```
┌─────────────────────────────────────────────────────────────────┐
│  Memory Layout                                                  │
│                                                                 │
│  ┌───────────────────┐  High Address (0x20082000)               │
│  │      STACK        │  ← Automatic/local variables             │
│  │   (grows down)    │    Created/destroyed per function        │
│  ├───────────────────┤                                          │
│  │                   │                                          │
│  │    (free space)   │                                          │
│  │                   │                                          │
│  ├───────────────────┤                                          │
│  │       HEAP        │  ← Dynamic allocation (malloc/free)      │
│  │    (grows up)     │                                          │
│  ├───────────────────┤                                          │
│  │   .bss section    │  ← Uninitialized static/global vars      │
│  ├───────────────────┤                                          │
│  │   .data section   │  ← Initialized static/global vars        │
│  └───────────────────┘  Low Address (0x20000000)                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Key Point:** Static variables are NOT on the heap! They live in a fixed location in the `.data` section (if initialized) or `.bss` section (if uninitialized). This is different from heap memory which is dynamically allocated at runtime.

### What Happens with Overflow?

Since `static_fav_num` is a `uint8_t` (unsigned 8-bit), it can only hold values 0-255. What happens when it reaches 255 and we add 1?

```
255 + 1 = 256... but that doesn't fit in 8 bits!
Binary: 11111111 + 1 = 100000000 (9 bits)
The 9th bit is lost, so we get: 00000000 = 0
```

This is called **overflow** or **wrap-around**. The value "wraps" back to 0 and starts counting again!

---

## 📚 Part 2: Understanding GPIO Inputs

### Input vs Output

So far, we've used GPIO pins as **outputs** to control LEDs. Now we'll learn to use them as **inputs** to read button states!

```
┌─────────────────────────────────────────────────────────────────┐
│  GPIO Direction                                                 │
│                                                                 │
│  OUTPUT (what we've done before):                               │
│  ┌─────────┐                                                    │
│  │  Pico   │ ───────► LED                                       │
│  │ GPIO 16 │          (We control the LED)                      │
│  └─────────┘                                                    │
│                                                                 │
│  INPUT (new this week):                                         │
│  ┌─────────┐                                                    │
│  │  Pico   │ ◄─────── Button                                    │
│  │ GPIO 15 │          (We read the button state)                │
│  └─────────┘                                                    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The Floating Input Problem

When a GPIO pin is set as an input but nothing is connected, it's called a **floating input**. The voltage on the pin is undefined and can randomly read as HIGH (1) or LOW (0) due to electrical noise.

```
┌─────────────────────────────────────────────────────────────────┐
│  Floating Input = Random Values!                                │
│                                                                 │
│  GPIO Pin (no connection):                                      │
│    Reading 1: HIGH                                              │
│    Reading 2: LOW                                               │
│    Reading 3: HIGH                                              │
│    Reading 4: HIGH                                              │
│    Reading 5: LOW                                               │
│    (Completely unpredictable!)                                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Pull-Up and Pull-Down Resistors

To solve the floating input problem, we use **pull resistors**:

| Resistor Type | Default State | When Button Pressed |
| ------------- | ------------- | ------------------- |
| **Pull-Up**   | HIGH (1)      | LOW (0)             |
| **Pull-Down** | LOW (0)       | HIGH (1)            |

The Pico 2 has **internal** pull resistors that you can enable with software - no external components needed!

```
┌─────────────────────────────────────────────────────────────────┐
│  Pull-Up Resistor (what we're using)                            │
│                                                                 │
│     3.3V                                                        │
│       │                                                         │
│       ┴ (internal pull-up resistor)                             │
│       │                                                         │
│       ├──────► GPIO 15 (reads HIGH normally)                    │
│       │                                                         │
│     ┌─┴─┐                                                       │
│     │BTN│ ← Button connects GPIO to GND when pressed            │
│     └─┬─┘                                                       │
│       │                                                         │
│      GND                                                        │
│                                                                 │
│  Button NOT pressed: GPIO reads 1 (HIGH)                        │
│  Button PRESSED:     GPIO reads 0 (LOW)                         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### GPIO Input Functions

| Function                     | Purpose                                 |
| ---------------------------- | --------------------------------------- |
| `gpio_init(pin)`             | Initialize a GPIO pin for use           |
| `gpio_set_dir(pin, GPIO_IN)` | Set pin as INPUT                        |
| `gpio_pull_up(pin)`          | Enable internal pull-up resistor        |
| `gpio_pull_down(pin)`        | Enable internal pull-down resistor      |
| `gpio_get(pin)`              | Read the current state (returns 0 or 1) |

### The Ternary Operator

The code uses a **ternary operator** to control the LED based on button state:

```c
gpio_put(LED_GPIO, pressed ? 0 : 1);
```

This is a compact if-else statement:
- If `pressed` is **true (1)**: output `0` (LED OFF... wait, that seems backwards!)
- If `pressed` is **false (0)**: output `1` (LED ON)

**Why is it inverted?** Because of the pull-up resistor!
- Button **released** → GPIO reads `1` → `pressed = 1` → output `0` → LED OFF
- Button **pressed** → GPIO reads `0` → `pressed = 0` → output `1` → LED ON

A clearer way to write this:
```c
gpio_put(LED_GPIO, !gpio_get(BUTTON_GPIO));
```

---

## 📚 Part 3: Understanding Compiler Optimizations

### Why Does Code Disappear?

When you compile code, the compiler tries to make it faster and smaller. This is called **optimization**. Sometimes the compiler removes code that it thinks has no effect!

**Example from our code:**
```c
while (true) {
    uint8_t regular_fav_num = 42; // Created
    regular_fav_num++;            // Incremented to 43
    // But then it's destroyed and recreated as 42 next loop!
}
```

The compiler sees that incrementing `regular_fav_num` has no lasting effect (because it's recreated as 42 each loop), so it may **optimize away** the increment operation entirely!

### Function Inlining

Sometimes the compiler **inlines** functions, meaning it replaces a function call with the function's code directly. 

**Original code:**
```c
gpio_pull_up(BUTTON_GPIO);
```

**What the compiler might do:**
```c
// Instead of calling gpio_pull_up, it calls the underlying function:
gpio_set_pulls(BUTTON_GPIO, true, false);
```

This is why when you look for `gpio_pull_up` in the binary, you might find `gpio_set_pulls` instead!

---

## 📚 Part 4: Setting Up Your Environment

### Prerequisites

Before we start, make sure you have:
1. A Raspberry Pi Pico 2 board
2. A Raspberry Pi Pico Debug Probe
3. OpenOCD installed and configured
4. GDB (`arm-none-eabi-gdb`) installed
5. Python installed (for UF2 conversion)
6. A serial monitor (PuTTY, minicom, or screen)
7. A push button connected to GPIO 15
8. An LED connected to GPIO 16 (or use the breadboard LED)
9. A hex editor (HxD, ImHex, or similar)
10. The sample project: `0x0014_static-variables`

### Hardware Setup

Connect your button like this:
- One side of button → GPIO 15
- Other side of button → GND

The internal pull-up resistor provides the 3.3V connection, so you only need to connect to GND!

```
┌─────────────────────────────────────────────────────────────────┐
│  Breadboard Wiring                                              │
│                                                                 │
│  Pico 2                                                         │
│  ┌──────────┐                                                   │
│  │          │                                                   │
│  │ GPIO 15  │────────┐                                          │
│  │          │        │                                          │
│  │ GPIO 16  │────────┼───► LED (with resistor to GND)           │
│  │          │        │                                          │
│  │   GND    │────────┼───┐                                      │
│  │          │        │   │                                      │
│  └──────────┘      ┌─┴─┐ │                                      │
│                    │BTN│─┘                                      │
│                    └───┘                                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Project Structure

```
Embedded-Hacking/
├── 0x0014_static-variables/
│   ├── build/
│   │   ├── 0x0014_static-variables.uf2
│   │   └── 0x0014_static-variables.elf
│   └── 0x0014_static-variables.c
└── uf2conv.py
```

---

## 🔬 Part 5: Hands-On Tutorial - Static Variables and GPIO Input

### Step 1: Review the Source Code

Let's examine the static variables code:

**File: `0x0014_static-variables.c`**

```c
#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();
    
    const uint BUTTON_GPIO = 15;
    const uint LED_GPIO = 16;
    bool pressed = 0;
    
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);
    
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
    
    while (true) {
        uint8_t regular_fav_num = 42;
        static uint8_t static_fav_num = 42;
        
        printf("regular_fav_num: %d\r\n", regular_fav_num);
        printf("static_fav_num: %d\r\n", static_fav_num);
        
        regular_fav_num++;
        static_fav_num++;
        
        pressed = gpio_get(BUTTON_GPIO);
        gpio_put(LED_GPIO, pressed ? 0 : 1);
    }
}
```

**What this code does:**

1. **Line 6-8:** Defines constants for button (GPIO 15) and LED (GPIO 16) pins
2. **Line 10-12:** Sets up GPIO 15 as input with internal pull-up resistor
3. **Line 14-15:** Sets up GPIO 16 as output for the LED
4. **Line 18-19:** Creates two variables:
   - `regular_fav_num` - a normal local variable (recreated each loop)
   - `static_fav_num` - a static variable (persists across loops)
5. **Line 21-22:** Prints both values to the serial terminal
6. **Line 24-25:** Increments both values
7. **Line 27-28:** Reads button and controls LED accordingly

### Step 2: Flash the Binary to Your Pico 2

1. Hold the BOOTSEL button on your Pico 2
2. Plug in the USB cable (while holding BOOTSEL)
3. Release BOOTSEL - a drive called "RPI-RP2" appears
4. Drag and drop `0x0014_static-variables.uf2` onto the drive
5. The Pico will reboot and start running!

### Step 3: Open Your Serial Monitor

Open PuTTY, minicom, or screen and connect to your Pico's serial port.

**You should see output like this:**

```
...
regular_fav_num: 42
static_fav_num: 42
regular_fav_num: 42
static_fav_num: 43
regular_fav_num: 42
static_fav_num: 44
regular_fav_num: 42
static_fav_num: 45
...
```

**Notice the difference:**
- `regular_fav_num` stays at 42 every time (it's recreated each loop)
- `static_fav_num` increases each time (it persists and remembers its value)

### Step 4: Test the Button

Now test the button behavior:
- **Button NOT pressed:** LED should be ON (because of the inverted logic)
- **Button PRESSED:** LED should turn OFF

Wait... that seems backwards from what you'd expect! That's because of the pull-up resistor and the ternary operator. We'll hack this later to make it more intuitive!

### Step 5: Watch for Overflow

Keep the program running and watch `static_fav_num`. After 255, you'll see:

```
static_fav_num: 254
static_fav_num: 255
static_fav_num: 0      ← Wrapped around!
static_fav_num: 1
static_fav_num: 2
...
```

This demonstrates unsigned integer overflow!

---

## 🔬 Part 6: Debugging with GDB (Dynamic Analysis)

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
arm-none-eabi-gdb build/0x0014_static-variables.elf
```

**Connect to target:**

```gdb
(gdb) target remote :3333
(gdb) monitor reset halt
```

### Step 6: Examine Main Function

Let's examine the main function at its entry point. First, disassemble from the start:

```
x/38i 0x10000234
```

You should see output like:

```
   0x10000234 <+0>:     push    {r4, lr}
   0x10000236 <+2>:     bl      0x10003014 <stdio_init_all>
   0x1000023a <+6>:     movs    r0, #15
   0x1000023c <+8>:     bl      0x10000300 <gpio_init>
   0x10000240 <+12>:    movs    r0, #15
   0x10000242 <+14>:    mov.w   r3, #0
   0x10000246 <+18>:    mcrr    0, 4, r0, r3, cr4
   0x1000024a <+22>:    movs    r2, #0
   0x1000024c <+24>:    movs    r1, #1
   0x1000024e <+26>:    bl      0x100002d8 <gpio_set_pulls>
   0x10000252 <+30>:    movs    r0, #16
   0x10000254 <+32>:    bl      0x10000300 <gpio_init>
   0x10000258 <+36>:    movs    r3, #16
   0x1000025a <+38>:    mov.w   r2, #1
   0x1000025e <+42>:    mcrr    0, 4, r3, r2, cr4
   0x10000262 <+46>:    ldr     r4, [pc, #44]   @ (0x10000290 <main+92>)
   0x10000264 <+48>:    movs    r1, #42 @ 0x2a
   0x10000266 <+50>:    ldr     r0, [pc, #44]   @ (0x10000294 <main+96>)
   0x10000268 <+52>:    bl      0x100031a4 <__wrap_printf>
   0x1000026c <+56>:    ldrb    r1, [r4, #0]
   0x1000026e <+58>:    ldr     r0, [pc, #40]   @ (0x10000298 <main+100>)
   0x10000270 <+60>:    bl      0x100031a4 <__wrap_printf>
   0x10000274 <+64>:    mov.w   r1, #3489660928 @ 0xd0000000
   0x10000278 <+68>:    ldrb    r3, [r4, #0]
   0x1000027a <+70>:    movs    r2, #16
   0x1000027c <+72>:    adds    r3, #1
   0x1000027e <+74>:    strb    r3, [r4, #0]
   0x10000280 <+76>:    ldr     r3, [r1, #4]
   0x10000282 <+78>:    ubfx    r3, r3, #15, #1
   0x10000286 <+82>:    eor.w   r3, r3, #1
   0x1000028a <+86>:    mcrr    0, 4, r2, r3, cr0
   0x1000028e <+90>:    b.n     0x10000264 <main+48>
   0x10000290 <+92>:    lsls    r0, r5, #22
   0x10000292 <+94>:    movs    r0, #0
   0x10000294 <+96>:    adds    r5, #96 @ 0x60
   0x10000296 <+98>:    asrs    r0, r0, #32
   0x10000298 <+100>:   adds    r5, #120        @ 0x78
   0x1000029a <+102>:   asrs    r0, r0, #32
```

### Step 7: Set a Breakpoint at Main

```
b *0x10000234
c
```

GDB responds:
```
Breakpoint 1 at 0x10000234: file C:/Users/flare-vm/Desktop/Embedded-Hacking-main/0x0014_static-variables/0x0014_static-variables.c, line 5.
Note: automatically using hardware breakpoints for read-only addresses.
(gdb) c
Continuing.

Thread 1 "rp2350.cm0" hit Breakpoint 1, main ()
    at C:/Users/flare-vm/Desktop/Embedded-Hacking-main/0x0014_static-variables/0x0014_static-variables.c:5
5           stdio_init_all();
```

> ⚠️ **Note:** If GDB says `The program is not being run.` when you type `c`, the target hasn't been started yet. Use `monitor reset halt` first, then `c` to continue to your breakpoint.

### Step 8: Examine the Static Variable Location

Static variables live at fixed RAM addresses. But how do we find that address? Look at the first instruction in the disassembly from Step 6:

```
0x10000262:    ldr r4, [pc, #44]   @ (0x10000290 <main+92>)
```

This loads `r4` from the **literal pool** at address `0x10000290`. The literal pool stores constants that are too large for immediate encoding — in this case, a 32-bit RAM address. Let's examine what's stored there:

```gdb
(gdb) x/1wx 0x10000290
0x10000290 <main+92>:   0x200005a8
```

That's `0x200005a8` — the RAM address of `static_fav_num`! The compiler placed this address in the literal pool because it can't encode a full 32-bit address in a single Thumb instruction.

> 💡 **Why did the disassembly at `0x10000290` show `lsls r0, r5, #22` instead?** Because `x/i` (disassemble) interprets raw data as instructions. The bytes `A8 05 00 20` at that address are the little-endian encoding of `0x200005A8`, but GDB's disassembler doesn't know it's data — it tries to decode it as a Thumb instruction. Using `x/wx` (examine as word) shows the actual value.

### Step 9: Step Through the Loop

Set a breakpoint at the start of the loop and step through:

```gdb
(gdb) b *0x10000264
(gdb) c
```

Now use `si` (step instruction) to execute one instruction at a time:

```gdb
(gdb) si
```

Watch how the static variable gets loaded (`ldrb`), incremented (`adds`), and stored back (`strb`).

### Step 10: Examine Register Values

After stepping to `0x10000262` or later, check the registers:

```gdb
(gdb) i r
```

Pay attention to:
- `r4` — Should hold `0x200005a8` (static variable's RAM address, loaded from literal pool)
- `r1` — Used for `printf` arguments (holds `42` or the static variable value)
- `r3` — Used for load/increment/store of the static variable
- `pc` — Program counter (current instruction address)

### Step 11: Watch the Static Variable Change

Now that we know the static variable lives at `0x200005a8`, examine it directly:

```gdb
(gdb) x/1db 0x200005a8
0x200005a8:     42
```

Step through a full loop iteration (back to `0x10000264`) and re-examine:

```gdb
(gdb) c
(gdb) x/1db 0x200005a8
0x200005a8:     43
```

The value incremented from 42 to 43! Each loop iteration, the `adds r3, #1` at `0x1000027c` bumps it by 1, and `strb r3, [r4, #0]` at `0x1000027e` writes it back to RAM.

### Step 12: Examine GPIO State

Read the GPIO input register to see the button state:

```gdb
(gdb) x/1wx 0xd0000004
```

The SIO GPIO input register at `0xd0000004` shows the current state of all GPIO pins. Bit 15 corresponds to our button on GPIO 15. To extract just bit 15:

```gdb
(gdb) p/x (*(unsigned int *)0xd0000004 >> 15) & 1
```

- Returns `1` when button is **not pressed** (pull-up holds it HIGH)
- Returns `0` when button is **pressed** (connected to GND)

TRY IT!

---

## 🔬 Part 7: Understanding the Assembly

Now that we've explored the binary in GDB, let's make sense of the key patterns.

### Step 13: Analyze the Regular Variable

In GDB, examine the code at the start of the loop:

```gdb
(gdb) x/5i 0x10000262
```

Look for this instruction:

```
0x10000264:    movs r1, #42 @ 0x2a
```

This loads the value `0x2a` (42 in decimal) directly into register `r1` for the first `printf` call.

**Key insight:** The compiler **optimized away** the `regular_fav_num` variable entirely! Since it's always 42 when printed, the compiler just uses the constant `42` directly. The `regular_fav_num++` after the print is also removed because it has no observable effect.

### Step 14: Analyze the Static Variable

Examine the static variable operations in the second half of the loop body:

```gdb
(gdb) x/10i 0x10000274
```

Look for the load-increment-store pattern using `r4` (which holds the static variable's RAM address):

```
   0x10000274 <main+64>:        mov.w   r1, #3489660928 @ 0xd0000000
   0x10000278 <main+68>:        ldrb    r3, [r4, #0]
   0x1000027a <main+70>:        movs    r2, #16
   0x1000027c <main+72>:        adds    r3, #1
   0x1000027e <main+74>:        strb    r3, [r4, #0]
   0x10000280 <main+76>:        ldr     r3, [r1, #4]
   0x10000282 <main+78>:        ubfx    r3, r3, #15, #1
   0x10000286 <main+82>:        eor.w   r3, r3, #1
   0x1000028a <main+86>:        mcrr    0, 4, r2, r3, cr0
   0x1000028e <main+90>:        b.n     0x10000264 <main+48>
```

Note that `r4` was loaded earlier at `0x10000262` via `ldr r4, [pc, #44]` — this pulled the static variable's RAM address (`0x200005a8`) from the literal pool at `0x10000290`.

**Key insight:** The static variable lives at a **fixed RAM address** (`0x200005a8`). It's loaded, incremented, and stored back — unlike the regular variable which was optimized away!

Verify the static variable value which should be `43`:

```gdb
(gdb) x/1db 0x200005a8
```

### Step 15: Analyze the GPIO Logic

Examine the GPIO input/output code:

```gdb
(gdb) x/10i 0x10000274
```

Look for this sequence:

```
   0x10000274 <main+64>:        mov.w   r1, #3489660928 @ 0xd0000000
   0x10000278 <main+68>:        ldrb    r3, [r4, #0]
   0x1000027a <main+70>:        movs    r2, #16
   0x1000027c <main+72>:        adds    r3, #1
   0x1000027e <main+74>:        strb    r3, [r4, #0]
   0x10000280 <main+76>:        ldr     r3, [r1, #4]
   0x10000282 <main+78>:        ubfx    r3, r3, #15, #1
   0x10000286 <main+82>:        eor.w   r3, r3, #1
   0x1000028a <main+86>:        mcrr    0, 4, r2, r3, cr0
   0x1000028e <main+90>:        b.n     0x10000264 <main+48>
```

**Breaking this down:**

| Address        | Instruction                | Purpose                                              |
| -------------- | -------------------------- | ---------------------------------------------------- |
| `0x10000274`   | `mov.w r1, #0xd0000000`    | Load SIO (Single-cycle I/O) base address into `r1`   |
| `0x10000278`   | `ldrb r3, [r4, #0]`        | Load `static_fav_num` from RAM into `r3`             |
| `0x1000027a`   | `movs r2, #16`             | Load LED pin number (16) into `r2` for later         |
| `0x1000027c`   | `adds r3, #1`              | Increment `static_fav_num` by 1                      |
| `0x1000027e`   | `strb r3, [r4, #0]`        | Store incremented value back to RAM                  |
| `0x10000280`   | `ldr r3, [r1, #4]`         | Read GPIO input state (SIO_GPIO_IN at offset `0x04`) |
| `0x10000282`   | `ubfx r3, r3, #15, #1`     | Extract bit 15 (GPIO 15 = button)                    |
| `0x10000286`   | `eor.w r3, r3, #1`         | XOR with 1 to invert (implements `? 0 : 1`)          |
| `0x1000028a`   | `mcrr 0, 4, r2, r3, cr0`   | Write `r3` (button) and `r2` (pin 16) to GPIO output |
| `0x1000028e`   | `b.n 0x10000264`           | Loop back to start (`while (true)`)                  |

> 💡 **Notice how the compiler interleaves the static variable increment with the GPIO logic.** It loads the SIO base address (`r1`) *before* doing the increment, and sets up `r2 = 16` (LED pin) in between. This is called **instruction scheduling** — the compiler reorders instructions to avoid pipeline stalls while waiting for memory reads.

### Step 16: Find the Infinite Loop

The last instruction at `0x1000028e` is already covered in the table above:

```
0x1000028e:    b.n 0x10000264
```

This is an **unconditional branch** back to `0x10000264` (the `movs r1, #42` at the top of the loop) — this is the `while (true)` in our code! There is no `pop` or `bx lr` to return from `main` because the loop never exits.

---

## 🔬 Part 8: Hacking the Binary with a Hex Editor

Now for the fun part — we'll patch the `.bin` file directly using a hex editor!

> 💡 **Why a hex editor?** GDB **cannot write to flash memory** — the `0x10000000+` address range where program instructions live. Trying `set *(char *)0x10000264 = 0x2b` in GDB gives `Writing to flash memory forbidden in this context`. To make **permanent** patches that survive a power cycle, we edit the `.bin` file directly with a hex editor and re-flash it.

### Step 17: Open the Binary in a Hex Editor

1. Open **HxD** (or your preferred hex editor: ImHex, 010 Editor, etc.)
2. Click **File** → **Open**
3. Navigate to `C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0014_static-variables\build\`
4. Open `0x0014_static-variables.bin`

### Step 18: Calculate the File Offset

The binary is loaded at base address `0x10000000`. To find the file offset of any address:

```
file_offset = address - 0x10000000
```

For example:
- Address `0x10000264` → file offset `0x264` (612 in decimal)
- Address `0x10000286` → file offset `0x286` (646 in decimal)

### Step 19: Hack #1 — Change regular_fav_num from 42 to 43

From our GDB analysis, we know the instruction at `0x10000264` is:

```
movs r1, #0x2a    →    bytes: 2a 21
```

To change the value from 42 (`0x2a`) to 43 (`0x2b`):

1. In HxD, open `C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0014_static-variables\build\0x0014_static-variables.bin`
2. Press **Ctrl+G** (Go to offset)
3. Enter offset: `264`
4. You should see the byte `2A` at this position
5. Change `2A` to `2B`
6. The instruction is now `movs r1, #0x2b` (43 in decimal)

> 🔍 **How Thumb encoding works:** In `movs r1, #imm8`, the immediate value is the first byte, and the opcode `21` is the second byte. So the bytes `2a 21` encode `movs r1, #0x2a`.

### Step 20: Hack #2 — Invert the Button Logic

#### Understand the Encoding

From GDB, we found the `eor.w r3, r3, #1` instruction at `0x10000286` that inverts the button value. Examine the exact bytes:

```gdb
(gdb) x/4bx 0x10000286
0x10000286 <main+82>:   0x83    0xf0    0x01    0x03
```

This is the 32-bit Thumb-2 encoding of `eor.w r3, r3, #1`. The bytes break down as:

```
┌─────────────────────────────────────────────────────────────────┐
│  eor.w r3, r3, #1  →  bytes: 83 F0 01 03                        │
│                                                                 │
│  Byte 0: 0x83  ─┐                                               │
│  Byte 1: 0xF0  ─┘  First halfword (opcode + source register)    │
│  Byte 2: 0x01  ──── Immediate value (#1) ← CHANGE THIS          │
│  Byte 3: 0x03  ──── Destination register (r3)                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

To change `eor.w r3, r3, #1` to `eor.w r3, r3, #0` (making XOR do nothing):

The file offset is `0x10000286 - 0x10000000 = 0x286`. The immediate byte is the 3rd byte of the instruction, so: `0x286 + 2 = 0x288`.

To change `eor.w r3, r3, #1` to `eor.w r3, r3, #0`:

1. In HxD, press **Ctrl+G** (Go to offset)
2. Enter offset: `288` (the third byte of the 4-byte instruction)
3. You should see the byte `01` at this position
4. Change `01` to `00`

> 🔍 **Why offset `0x288` and not `0x286`?** The immediate value `#1` is in the **third byte** of the 4-byte instruction. The instruction starts at file offset `0x286`, so the immediate byte is at `0x286 + 2 = 0x288`.

Now the logic is permanently changed:
- Button released (input = 1): `1 XOR 0 = 1` → LED **ON**
- Button pressed (input = 0): `0 XOR 0 = 0` → LED **OFF**

This is the **opposite** of the original behavior!

### Step 21: Save the Patched Binary

1. Click **File** → **Save As**
2. Save as `0x0014_static-variables-h.bin` in the build directory
3. Close the hex editor

---

## 🔬 Part 9: Converting and Flashing the Hacked Binary

### Step 22: Convert to UF2 Format

Open a terminal and navigate to your project directory:

```powershell
cd C:\Users\flare-vm\Desktop\Embedded-Hacking-main\0x0014_static-variables
```

Run the conversion command:

```powershell
python ..\uf2conv.py build\0x0014_static-variables-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
```

**What this command means:**
- `uf2conv.py` = the conversion script (in the parent `Embedded-Hacking-main` directory)
- `--base 0x10000000` = the XIP base address where code runs from
- `--family 0xe48bff59` = the RP2350 family ID
- `--output build\hacked.uf2` = the output filename

### Step 23: Flash the Hacked Binary

1. Hold BOOTSEL and plug in your Pico 2
2. Drag and drop `hacked.uf2` onto the RPI-RP2 drive
3. Open your serial monitor

### Step 24: Verify the Hacks

**Check the serial output:**
```
regular_fav_num: 43    ← Changed from 42!
static_fav_num: 42
regular_fav_num: 43
static_fav_num: 43
...
```

**Check the LED behavior:**
- LED should now be **ON by default** (when button is NOT pressed)
- LED should turn **OFF** when you press the button

🎉 **BOOM! We successfully:**
1. Changed the printed value from 42 to 43
2. Inverted the LED/button logic

---

## 📊 Part 10: Summary and Review

### What We Accomplished

1. **Learned about static variables** - How they persist across function calls and loop iterations
2. **Understood memory layout** - Stack vs static storage vs heap
3. **Configured GPIO inputs** - Using pull-up resistors and reading button states
4. **Analyzed compiled code in GDB** - Saw how the compiler optimizes code
5. **Discovered function inlining** - `gpio_pull_up` became `gpio_set_pulls`
6. **Hacked variable values** - Changed 42 to 43 using a hex editor
7. **Inverted GPIO logic** - Made LED behavior opposite

### Static vs Automatic Variables

| Aspect             | Automatic (Regular)      | Static                      |
| ------------------ | ------------------------ | --------------------------- |
| **Storage**        | Stack                    | Static storage (.data/.bss) |
| **Lifetime**       | Block/function scope     | Entire program              |
| **Initialization** | Every time block entered | Once at program start       |
| **Persistence**    | Lost when scope exits    | Retained between calls      |
| **Compiler view**  | May be optimized away    | Always has memory location  |

### GPIO Input Configuration

```
┌─────────────────────────────────────────────────────────────────┐
│  GPIO Input Setup Steps                                         │
│                                                                 │
│  1. gpio_init(pin)                   - Initialize the pin       │
│  2. gpio_set_dir(pin, GPIO_IN)       - Set as input             │
│  3. gpio_pull_up(pin)                - Enable pull-up           │
│     OR gpio_pull_down(pin)           - OR enable pull-down      │
│  4. gpio_get(pin)                    - Read the state           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The Binary Hacking Workflow

```
┌─────────────────────────────────────────────────────────────────┐
│  1. Analyze the binary with GDB                                 │
│     - Disassemble functions with x/Ni                           │
│     - Identify key instructions and addresses                   │
├─────────────────────────────────────────────────────────────────┤
│  2. Understand compiler optimizations                           │
│     - Some functions get inlined (gpio_pull_up → gpio_set_pulls)│
│     - Some variables are optimized away                         │
├─────────────────────────────────────────────────────────────────┤
│  3. Calculate file offsets                                      │
│     - file_offset = address - 0x10000000                        │
├─────────────────────────────────────────────────────────────────┤
│  4. Patch the .bin file with a hex editor                       │
│     - Open the .bin file in HxD / ImHex                         │
│     - Go to the calculated offset                               │
│     - Change the target byte(s)                                 │
├─────────────────────────────────────────────────────────────────┤
│  5. Convert to UF2                                              │
│     python uf2conv.py file.bin --base 0x10000000                │
│       --family 0xe48bff59 --output hacked.uf2                   │
├─────────────────────────────────────────────────────────────────┤
│  6. Flash and verify                                            │
│     - Hold BOOTSEL, plug in, drag UF2                           │
│     - Check serial output and button/LED behavior               │
└─────────────────────────────────────────────────────────────────┘
```

### Key Memory Addresses

| Address      | Description                         |
| ------------ | ----------------------------------- |
| `0x10000234` | Typical main() entry point          |
| `0x10003014` | stdio_init_all() function           |
| `0x200005a8` | Static variable storage (example)   |
| `0xd0000000` | SIO (Single-cycle I/O) base address |

---

## ✅ Practice Exercises

### Exercise 1: Change Static Variable Initial Value
The static variable starts at 42. Hack the binary to make it start at 100 instead.

**Hint:** Find where `DAT_200005a8` is initialized in the .data section.

### Exercise 2: Make the LED Blink
Instead of responding to button presses, hack the binary to make the LED blink continuously.

**Hint:** You'll need to change the GPIO output logic to toggle instead of following button state.

### Exercise 3: Reverse Engineer gpio_set_pulls
Using GDB, disassemble the `gpio_set_pulls` function and figure out what registers it writes to.

**Hint:** Look for writes to addresses around `0x40038000` (PADS_BANK0).

### Exercise 4: Add a Second Static Variable
If you had two static variables, where would they be stored in memory? Would they be next to each other?

**Hint:** Static variables in the same compilation unit are typically placed consecutively in the .data section.

### Exercise 5: Overflow Faster
The static variable overflows after 255 iterations. Can you hack it to overflow sooner?

**Hint:** Change the increment from `+1` to `+10` by modifying the `adds r3,#0x1` instruction.

---

## 🎓 Key Takeaways

1. **Static variables persist** - They keep their value between function calls and loop iterations.

2. **Static storage ≠ heap** - Static variables are in a fixed location, not dynamically allocated.

3. **Compilers optimize aggressively** - Regular variables may be optimized away if the compiler sees no effect.

4. **Function inlining is common** - `gpio_pull_up` becomes `gpio_set_pulls` in the binary.

5. **Pull-up resistors invert logic** - Button pressed = LOW, button released = HIGH.

6. **XOR is useful for inverting** - `eor r3,r3,#0x1` flips a bit between 0 and 1.

7. **Static variables have fixed addresses** - You can find them in the .data section at known RAM addresses.

8. **Overflow wraps around** - A `uint8_t` at 255 becomes 0 when incremented.

9. **UBFX extracts bits** - Used to read a single GPIO pin from a register.

10. **Binary patching is powerful** - Change values and logic without source code!

---

## 📖 Glossary

| Term                  | Definition                                                       |
| --------------------- | ---------------------------------------------------------------- |
| **Automatic**         | Variable that's created and destroyed automatically (local vars) |
| **eor/XOR**           | Exclusive OR - flips bits where operands differ                  |
| **Floating Input**    | GPIO input with undefined voltage (reads random values)          |
| **Function Inlining** | Compiler replaces function call with the function's code         |
| **gpio_get**          | Function to read the current state of a GPIO pin                 |
| **Heap**              | Memory area for dynamic allocation (malloc/free)                 |
| **Overflow**          | When a value exceeds its type's maximum and wraps around         |
| **Pull-Down**         | Resistor that holds a pin LOW when nothing drives it             |
| **Pull-Up**           | Resistor that holds a pin HIGH when nothing drives it            |
| **SIO**               | Single-cycle I/O - fast GPIO access on RP2350                    |
| **Stack**             | Memory area for local variables and function call frames         |
| **Static Storage**    | Fixed memory area for static and global variables                |
| **Static Variable**   | Variable declared with `static` that persists across calls       |
| **Ternary Operator**  | `condition ? value_if_true : value_if_false`                     |
| **UBFX**              | Unsigned Bit Field Extract - extracts bits from a register       |
| **Varargs**           | Variable arguments - functions that take unlimited parameters    |

---

## 🔗 Additional Resources

### GPIO Input Reference

| Function                      | Purpose                    |
| ----------------------------- | -------------------------- |
| `gpio_init(pin)`              | Initialize GPIO pin        |
| `gpio_set_dir(pin, GPIO_IN)`  | Set pin as input           |
| `gpio_set_dir(pin, GPIO_OUT)` | Set pin as output          |
| `gpio_pull_up(pin)`           | Enable internal pull-up    |
| `gpio_pull_down(pin)`         | Enable internal pull-down  |
| `gpio_disable_pulls(pin)`     | Disable all pull resistors |
| `gpio_get(pin)`               | Read pin state (0 or 1)    |
| `gpio_put(pin, value)`        | Set pin output (0 or 1)    |

### Key Assembly Instructions

| Instruction             | Description                                  |
| ----------------------- | -------------------------------------------- |
| `movs rN, #imm`         | Move immediate value to register             |
| `ldrb rN, [rM, #off]`   | Load byte from memory                        |
| `strb rN, [rM, #off]`   | Store byte to memory                         |
| `adds rN, #imm`         | Add immediate value to register              |
| `eor rN, rM, #imm`      | Exclusive OR (XOR) with immediate            |
| `ubfx rN, rM, #lsb, #w` | Extract unsigned bit field                   |
| `mcrr p0, ...`          | Move to coprocessor (GPIO control on RP2350) |
| `b LABEL`               | Unconditional branch (jump)                  |

### Memory Map Quick Reference

| Address Range         | Description                    |
| --------------------- | ------------------------------ |
| `0x10000000`          | XIP Flash (code execution)     |
| `0x20000000-200005xx` | SRAM (.data section)           |
| `0x20082000`          | Stack top (initial SP)         |
| `0x40038000`          | PADS_BANK0 (pad configuration) |
| `0xd0000000`          | SIO (single-cycle I/O)         |

---

**Remember:** Static variables are your friends when you need to remember values across function calls. But they also make your program's behavior more complex to analyze - which is exactly why we practice reverse engineering!

Happy hacking! 🔧

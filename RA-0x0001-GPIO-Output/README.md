# RP2350 GPIO Coprocessor Blink - Detailed Documentation

## Overview

This is a **bare-metal assembly program** for the **Raspberry Pi RP2350** microcontroller that demonstrates the use of the **ARM Cortex-M33 coprocessor interface** to control GPIO pins. Unlike traditional GPIO control using direct memory-mapped register writes, this version uses the RP2350's **hardware GPIO coprocessor (CP0)** with **MCRR (Move to Coprocessor from two ARM Registers)** instructions.

The program blinks an LED connected to **GPIO16** with a 500ms on/off cycle using the specialized coprocessor instructions for high-performance GPIO operations.

---

## What Makes This Special?

### Traditional GPIO Control vs Coprocessor Control

**Traditional Method (Direct Register Access):**
```assembly
LDR  R0, =SIO_BASE           ; Load SIO base address
LDR  R1, =GPIO16_BIT         ; Load GPIO bit mask
STR  R1, [R0, #GPIO_OUT_SET] ; Write to memory-mapped register
```

**Coprocessor Method (This Implementation):**
```assembly
MOVS R4, #16                 ; GPIO pin number
MOVS R5, #1                  ; Value to write
MCRR p0, #4, R4, R5, c0      ; Single coprocessor instruction
```

### Advantages of the Coprocessor Approach:
1. **Faster execution** - Single instruction instead of multiple memory operations
2. **More atomic** - Less chance of race conditions
3. **Hardware-accelerated** - Direct hardware interface
4. **Cleaner code** - No need to manage bit masks and base addresses
5. **RP2350-specific** - Takes advantage of advanced features

---

## Hardware Requirements

- **RP2350-based board** (e.g., Raspberry Pi Pico 2)
- **LED** connected to **GPIO16** (or use built-in LED if available)
- Optional: Current-limiting resistor (typically 330Ω - 1kΩ)
- **USB cable** for programming

### GPIO16 Connection
```
RP2350 GPIO16 ----[330Ω]----[LED]---- GND
                  (Resistor)  (Anode to GPIO, Cathode to GND)
```

---

## Technical Details

### Coprocessor Interface

The RP2350 implements a **custom GPIO coprocessor (CP0)** accessible through the ARM Cortex-M33's coprocessor interface. This is documented in the RP2350 datasheet.

#### MCRR Instruction Format
```
MCRR p0, #opcode, Rt, Rt2, CRm
```

Where:
- `p0` = Coprocessor 0 (GPIO coprocessor)
- `#opcode` = Operation code (4 for GPIO operations)
- `Rt` = First source register (GPIO pin number in R4)
- `Rt2` = Second source register (value in R5)
- `CRm` = Coprocessor register (c0 for OUT, c4 for OE)

### GPIO Coprocessor Operations Used

#### 1. `gpioc_bit_out_put(gpio, value)`
**Purpose:** Set or clear a GPIO output pin  
**Encoding:** `MCRR p0, #4, R4, R5, c0`  
**Parameters:**
- R4 = GPIO pin number (16 in our case)
- R5 = Output value (0 or 1)

**Example:**
```assembly
MOVS R4, #16        ; GPIO16
MOVS R5, #1         ; Set HIGH
MCRR p0, #4, R4, R5, c0
```

#### 2. `gpioc_bit_oe_put(gpio, enable)`
**Purpose:** Set output enable for a GPIO pin  
**Encoding:** `MCRR p0, #4, R4, R5, c4`  
**Parameters:**
- R4 = GPIO pin number (16 in our case)
- R5 = Output enable (1 to enable output)

**Example:**
```assembly
MOVS R4, #16        ; GPIO16
MOVS R5, #1         ; Enable output
MCRR p0, #4, R4, R5, c4
```

---

## Memory Map & Register Addresses

### Critical Memory Addresses Used

| Address      | Register                          | Purpose                              |
|--------------|-----------------------------------|--------------------------------------|
| `0xE000ED88` | CPACR (Coprocessor Access Control)| Enable CP0 access                    |
| `0x40038044` | PADS_BANK0_GPIO16                 | Pad configuration for GPIO16         |
| `0x40028084` | IO_BANK0_GPIO16_CTRL              | Function select for GPIO16           |
| `0x20082000` | STACK_TOP                         | Initial stack pointer                |
| `0x2007A000` | STACK_LIMIT                       | Stack limit for overflow protection  |

### Register Details

#### CPACR (Coprocessor Access Control Register) - 0xE000ED88
```
Bits [1:0] - CP0 access privileges
  00 = No access
  01 = Privileged access only
  10 = Reserved
  11 = Full access (used in this code)
```

#### PADS_BANK0_GPIO16 - 0x40038044
```
Bit 8  (ISO)  - Isolate pad (0 = normal operation)
Bit 7  (OD)   - Output disable (0 = output enabled)
Bit 6  (IE)   - Input enable (1 = input buffer enabled)
Bits [5:4]    - Drive strength
Bits [3:2]    - Slew rate
Bit 1  (PDE)  - Pull-down enable
Bit 0  (PUE)  - Pull-up enable
```

#### IO_BANK0_GPIO16_CTRL - 0x40028084
```
Bits [4:0] (FUNCSEL) - Function select
  0 = JTAG (TDI)
  1 = SPI
  2 = UART
  3 = I2C
  4 = PWM
  5 = SIO (Software controlled I/O) ← Used for GPIO
  6-8 = PIO
  9 = USB
  31 = NULL
```

---

## Code Structure & Execution Flow

### 1. Vector Table (`.vectors` section)
```assembly
.section .vectors, "ax"
.align 2
.global _vectors
_vectors:
    .word STACK_TOP              /* 0x20082000 - Initial Stack Pointer */
    .word Reset_Handler + 1      /* Reset Handler (Thumb bit set) */
```

**Why `+ 1`?**  
The Cortex-M33 uses the Thumb instruction set exclusively. The LSB (least significant bit) of function pointers must be set to 1 to indicate Thumb mode. This is automatically handled by the processor.

### 2. Stack Initialization
```assembly
Reset_Handler:
    /* Initialize stack pointers for Cortex-M33 */
    LDR  R0, =STACK_TOP
    MSR  PSP, R0                 /* Process Stack Pointer */
    
    LDR  R0, =STACK_LIMIT
    MSR  MSPLIM, R0              /* Main Stack Pointer Limit */
    MSR  PSPLIM, R0              /* Process Stack Pointer Limit */
    
    LDR  R0, =STACK_TOP
    MSR  MSP, R0                 /* Main Stack Pointer */
```

**Cortex-M33 Stack Features:**
- **Dual stack pointers:** MSP (Main) and PSP (Process)
- **Stack limit registers:** MSPLIM and PSPLIM for overflow detection
- **Security feature:** Prevents stack overflow from corrupting memory

### 3. Coprocessor Access Enablement
```assembly
    /* Enable coprocessor access - set CP0 to full access */
    LDR  R0, =0xE000ED88         /* CPACR address */
    LDR  R1, [R0]                /* Read current value */
    ORR  R1, R1, #0x3            /* Set CP0 bits [1:0] to 11 (full access) */
    STR  R1, [R0]                /* Write back */
    DSB                          /* Data Synchronization Barrier */
    ISB                          /* Instruction Synchronization Barrier */
```

**Critical Step:** Without this, MCRR instructions will trigger a **UsageFault** exception!

**Memory Barriers:**
- `DSB` - Ensures all memory operations complete before proceeding
- `ISB` - Flushes instruction pipeline to ensure new settings take effect

### 4. GPIO Pad Configuration
```assembly
    /* Configure pad for GPIO16 */
    LDR  R3, =0x40038044         /* &pads_bank0_hw->io[16] */
    LDR  R2, [R3]                /* load current config */
    BIC  R2, R2, #0x80           /* clear OD (bit 7) - enable output */
    ORR  R2, R2, #0x40           /* set IE (bit 6) - enable input buffer */
    STR  R2, [R3]                /* store updated config */
```

**What this does:**
- Clears OD (Output Disable) bit → Allows pin to drive output
- Sets IE (Input Enable) bit → Enables input buffer for reading
- Preserves other settings (pull-ups, drive strength, etc.)

### 5. Function Selection
```assembly
    /* Set GPIO16 function to SIO (Software I/O) */
    LDR  R3, =0x40028084         /* &io_bank0_hw->io[16].ctrl */
    LDR  R2, [R3]                /* load current config */
    BIC  R2, R2, #0x1F           /* clear FUNCSEL bits [4:0] */
    ORR  R2, R2, #5              /* set FUNCSEL = 5 (SIO) */
    STR  R2, [R3]                /* store updated config */
```

**Function 5 (SIO):**
- Routes GPIO to software control
- Required before using coprocessor GPIO operations
- Other functions route to hardware peripherals (UART, SPI, etc.)

### 6. Pad Isolation
```assembly
    /* Un-isolate the pad */
    LDR  R3, =0x40038044         /* &pads_bank0_hw->io[16] */
    LDR  R2, [R3]                /* load current config */
    BIC  R2, R2, #0x100          /* clear ISO bit (bit 8) */
    STR  R2, [R3]                /* store updated config */
```

**ISO bit:**
- When set (1): Pad is isolated from peripherals
- When clear (0): Pad is connected and functional
- Must be cleared for GPIO to work

### 7. Enable GPIO Output
```assembly
    /* Enable output using coprocessor */
    MOVS R4, #16                 /* GPIO16 */
    MOVS R5, #1                  /* Enable output */
    MCRR p0, #4, R4, R5, c4      /* gpioc_bit_oe_put(16, 1) */
```

**This is the first coprocessor instruction!**
- Sets the Output Enable (OE) bit for GPIO16
- Makes the pin drive its output value
- Uses coprocessor register c4 (OE control)

### 8. Main Blink Loop
```assembly
blink_loop:
    /* Turn LED ON */
    MOVS R4, #16                 /* GPIO16 */
    MOVS R5, #1                  /* HIGH */
    MCRR p0, #4, R4, R5, c0      /* gpioc_bit_out_put(16, 1) */
    
    /* Delay ~500ms */
    LDR  R2, =2000000            /* 2M cycles */
delay_on:
    SUBS R2, R2, #1              /* Decrement */
    BNE  delay_on                /* Loop until zero */
    
    /* Turn LED OFF */
    MOVS R4, #16                 /* GPIO16 */
    MOVS R5, #0                  /* LOW */
    MCRR p0, #4, R4, R5, c0      /* gpioc_bit_out_put(16, 0) */
    
    /* Delay ~500ms */
    LDR  R2, =2000000            /* 2M cycles */
delay_off:
    SUBS R2, R2, #1              /* Decrement */
    BNE  delay_off               /* Loop until zero */

    B    blink_loop              /* Repeat forever */
```

**Delay Calculation:**
- 2,000,000 cycles at ~4-12 MHz boot clock
- Approximately 500ms per delay
- Total period: ~1 second (1Hz blink rate)

---

## Build Process

### Prerequisites
1. **ARM GNU Toolchain** installed and in PATH:
   - `arm-none-eabi-as` (Assembler)
   - `arm-none-eabi-ld` (Linker)
   - `arm-none-eabi-objcopy` (Object copy utility)
2. **Python** (for UF2 conversion)
3. **uf2conv.py** script (located two directories up: `../../uf2conv.py`)

### Build Script (`build.bat`)
```batch
@echo off
REM Build script for RP2350 GPIO16 blink

echo Building GPIO16 blink...

REM Assemble main code
arm-none-eabi-as -mcpu=cortex-m33 -mthumb gpio16_blink.s -o gpio16_blink.o

REM Assemble image definition
arm-none-eabi-as -mcpu=cortex-m33 -mthumb image_def.s -o image_def.o

REM Link with linker script
arm-none-eabi-ld -T linker.ld gpio16_blink.o image_def.o -o gpio16_blink.elf

REM Create raw binary
arm-none-eabi-objcopy -O binary gpio16_blink.elf gpio16_blink.bin

REM Create UF2 bootloader format
python ..\..\uf2conv.py -b 0x10000000 -f 0xe48bff59 -o gpio16_blink.uf2 gpio16_blink.bin
```

### Build Command
```powershell
.\build.bat
```

### Build Flags Explained

**`-mcpu=cortex-m33`**
- Target the Cortex-M33 processor in RP2350
- Enables M33-specific features (stack limits, DSB/ISB instructions, coprocessor)

**`-mthumb`**
- Generate Thumb-2 instruction set (mandatory for Cortex-M)
- Provides 16-bit and 32-bit instruction mix for code density

**`-T linker.ld`**
- Use custom linker script to define memory layout
- Places code at flash start (0x10000000)

**UF2 Parameters:**
- `-b 0x10000000` - Base address (RP2350 flash start)
- `-f 0xe48bff59` - Family ID for RP2350
- `-o gpio16_blink.uf2` - Output filename

---

## Flashing Instructions

### Method 1: UF2 Bootloader (Easiest)

1. **Disconnect** the RP2350 from USB
2. **Hold the BOOTSEL button** on the board
3. **Connect USB cable** while holding BOOTSEL
4. **Release BOOTSEL** - Board appears as USB mass storage device
5. **Copy** `gpio16_blink.uf2` to the RP2350 drive
6. **Board automatically reboots** and starts running the program

### Method 2: OpenOCD with Debug Probe

If you have a debug probe (e.g., Raspberry Pi Debug Probe, CMSIS-DAP):

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg \
  -c "adapter speed 5000" \
  -c "program gpio16_blink.elf verify reset exit"
```

---

## Debugging & Troubleshooting

### Problem: LED doesn't blink

**Check 1: Coprocessor Access**
- Ensure CPACR is set correctly
- Without this, MCRR triggers UsageFault
- Verify DSB/ISB barriers are present

**Check 2: GPIO Configuration**
- Verify FUNCSEL is set to 5 (SIO)
- Check OD bit is cleared (output enabled)
- Ensure ISO bit is cleared (pad connected)

**Check 3: Hardware Connections**
- LED polarity (anode to GPIO, cathode to GND)
- Current-limiting resistor present
- GPIO16 physically connected

**Check 4: Clock Speed**
- Boot clock may vary (4-12 MHz typical)
- Delay timing depends on actual clock
- Adjust delay constant if needed

### Problem: Build fails

**Missing toolchain:**
```
'arm-none-eabi-as' is not recognized...
```
Solution: Install ARM GNU Toolchain and add to PATH

**Missing uf2conv.py:**
```
python: can't open file 'uf2conv.py'
```
Solution: Ensure uf2conv.py is in `../../` relative to build directory

**Assembly errors:**
```
Error: bad instruction 'mcrr'
```
Solution: Ensure `-mcpu=cortex-m33` flag is used (M33 supports coprocessor)

### Problem: Board doesn't appear as USB drive

**Solution 1:** Try different USB cable (must support data, not just power)  
**Solution 2:** Hold BOOTSEL earlier (before connecting USB)  
**Solution 3:** Check USB port functionality

---

## Performance Analysis

### Instruction Cycle Counts (Approximate)

**Traditional GPIO Write (3-4 instructions):**
```assembly
LDR  R0, =SIO_BASE      ; 2 cycles
LDR  R1, =GPIO16_BIT    ; 2 cycles
STR  R1, [R0, #offset]  ; 2 cycles
                        ; Total: ~6 cycles + memory latency
```

**Coprocessor GPIO Write (2 instructions):**
```assembly
MOVS R4, #16            ; 1 cycle
MOVS R5, #1             ; 1 cycle
MCRR p0, #4, R4, R5, c0 ; 1 cycle (hardware accelerated)
                        ; Total: ~3 cycles
```

**Performance Gain: ~50% faster execution**

### Memory Usage

| Section      | Size    | Location       |
|-------------|---------|----------------|
| Vector Table| 8 bytes | 0x10000000     |
| Code (.text)| ~140 bytes | 0x10000008  |
| Total Flash | ~148 bytes | Flash          |
| Stack       | 32 KB   | RAM (0x2007A000-0x20082000) |

**UF2 File Size:** 512 bytes (minimum UF2 block size)

---

## Advanced Topics

### Extending to Multiple GPIOs

To control multiple GPIOs with coprocessor:

```assembly
; Blink GPIO16 and GPIO17 alternately
blink_multi:
    ; Turn GPIO16 ON, GPIO17 OFF
    MOVS R4, #16
    MOVS R5, #1
    MCRR p0, #4, R4, R5, c0
    
    MOVS R4, #17
    MOVS R5, #0
    MCRR p0, #4, R4, R5, c0
    
    ; Delay...
    
    ; Turn GPIO16 OFF, GPIO17 ON
    MOVS R4, #16
    MOVS R5, #0
    MCRR p0, #4, R4, R5, c0
    
    MOVS R4, #17
    MOVS R5, #1
    MCRR p0, #4, R4, R5, c0
```

### Reading GPIO with Coprocessor

The RP2350 also supports GPIO read operations:

```assembly
; Read GPIO state using MRRC (Move to ARM Registers from Coprocessor)
MOVS R4, #16              ; GPIO to read
MRRC p0, #opcode, R4, R5, c0  ; Result in R5
```

*(Exact opcode may vary - consult RP2350 datasheet)*

### Interrupt-Driven Delays

For production code, replace busy-wait delays with timer interrupts:

```assembly
; Configure SysTick for 1ms interrupts
LDR  R0, =0xE000E010    ; SysTick base
LDR  R1, =4000          ; Reload value for 1ms at 4MHz
STR  R1, [R0, #4]       ; SYST_RVR
MOV  R1, #7             ; Enable + interrupt + clock
STR  R1, [R0, #0]       ; SYST_CSR
```

---

## Reference Documentation

### Official Documentation
1. **RP2350 Datasheet** - Complete hardware reference
   - Section on GPIO Coprocessor (CP0)
   - Register maps and bit definitions
   - Electrical characteristics

2. **ARM Cortex-M33 Technical Reference Manual**
   - Coprocessor interface specification
   - MCRR/MRRC instruction encoding
   - Stack limit and security features

3. **ARM Architecture Reference Manual (ARMv8-M)**
   - Thumb-2 instruction set
   - System control registers (CPACR, etc.)
   - Exception handling

### Instruction Reference

**MCRR (Move to Coprocessor from ARM Registers):**
```
MCRR coproc, opc1, Rt, Rt2, CRm
  coproc: p0-p15 (coprocessor number)
  opc1:   4-bit operation code
  Rt:     First source register
  Rt2:    Second source register
  CRm:    Coprocessor register
```

**DSB (Data Synchronization Barrier):**
- Ensures completion of memory operations
- Required after CPACR modification

**ISB (Instruction Synchronization Barrier):**
- Flushes instruction pipeline
- Required after system control changes

---

## Version History

### Version 1.0 (October 2025)
- Initial release
- Implements GPIO16 blink using coprocessor
- 500ms on/off cycle
- CPACR enablement for CP0 access
- Proper initialization sequence

---

## License & Credits

This code is provided as-is for educational and experimental purposes.

**Created for:** Bare-metal RP2350 development  
**Target Board:** Raspberry Pi Pico 2 (RP2350)  
**Architecture:** ARM Cortex-M33  
**Assembly Syntax:** GNU AS (Unified ARM Syntax)

---

## Future Enhancements

Potential improvements:
1. **Add interrupt-driven timing** instead of busy-wait loops
2. **Implement GPIO read operations** using MRRC
3. **Create PWM effects** using variable duty cycles
4. **Multi-GPIO patterns** (Knight Rider, binary counter, etc.)
5. **Add error handling** for fault exceptions
6. **Clock configuration** for precise timing
7. **Low-power modes** using WFI (Wait For Interrupt)

---

## Contact & Support

For issues, questions, or contributions related to this coprocessor implementation, please refer to:
- RP2350 community forums
- ARM Cortex-M33 documentation
- Raspberry Pi Pico SDK examples

**Happy coding with coprocessors!** 🚀

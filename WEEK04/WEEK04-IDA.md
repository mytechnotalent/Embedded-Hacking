# Week 4-IDA: IDA Pro — Resolve, Hack, and Patch the RP2350 (Raw `.bin`)

***

**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only.

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## What You'll Learn This Week

- Build the two lesson projects with `Release` and get both an `.elf` and a raw `.bin`
- Dump the **ELF symbol map** with `arm-none-eabi-nm` and use it as ground truth
- Load the raw `.bin` into IDA at `0x10000000`
- **Resolve the functions in the IDA GUI** using the ELF symbol map
- **Patch** the bytes that control the behavior, export the image, and export it

---

## How This Guide Works

The build produces two files for each project:

| File | What it is | How we use it |
| ---- | ---------- | ------------- |
| `.elf` | The linked image with a full symbol table | Ground truth for every function address and name |
| `.bin` | The raw flash image, no headers, no symbols | The image we load into IDA and reverse |

The `.bin` is built **from** the `.elf`, so the ELF tells you exactly what is at every address. We use the ELF symbol map to resolve functions in IDA, and we reverse-engineer the raw `.bin` the way a real extracted firmware image is reversed.

> **Build `Release`, not `Debug`.** Every address in this guide matches the Week 4 lesson, and the Week 4 lesson is a `Release` build. `Release` optimizes the code the same way the original lesson was built: it folds `age = 42` away in Project 1 and inlines `blink_and_print` into `main` in Project 2. If you build `Debug`, the SDK function addresses move and Project 2 keeps a separate `blink_and_print`, so nothing lines up. Always build `Release` for this lesson.

The workflow is static-only: import the raw image, recover the code, decompile it, and patch the artifact:

d prove what the code does.
ge.
3. Resolve the functions in IDA using the ELF symbol map.
4. Patch the bytes, export, convert and export.

| Project | Prints | Also does | The hack |
| ------- | ------ | --------- | -------- |
| `0x0005_intro-to-variables` | `age: 43` | loops on `printf` | change `43` to `70` |
| `0x0008_uninitialized-variables` | `age: 0` | blinks the red LED on GPIO 16 | change `0` to `66`, move the LED to GPIO 17 |

> **Addresses come from your build.** Every address here is from the `Release` build produced in Step 3. Confirm against your own `.elf` with the command in Step 4.

---

## Part 1: Build and Get the Symbol Map

### Step 1: Install the toolchain

**Windows x64**

- Install the **Raspberry Pi Pico** extension in VS Code. It installs the ARM GNU toolchain, CMake, Ninja, and the Pico SDK.
- Install **IDA Pro** and complete its license activation.
- Install **PuTTY** for the serial monitor.

**macOS Apple Silicon**


All must report `arm64`. If any is `x86_64`, put the Apple Silicon prefix first for the session and check again:

## Glossary

| Term | Definition |
| ---- | ---------- |
| **`.bss`** | Section for uninitialized global variables; zeroed by startup code |
| **`.data`** | Section for initialized global variables; copied from flash to SRAM at boot |
| **`.elf`** | Linked image with the symbol table; the ground truth for addresses and names |
| **`.rodata`** | Read-only section for constants and string literals; stays in flash |
| **GPIO** | General Purpose Input/Output — controllable pins on the microcontroller |
ly flash |
| **Inlining** | The optimizer replacing a function call with the function body; why `blink_and_print` disappears in `Release` |
| **Literal pool** | A block of 32-bit constants that Thumb-2 code reaches with PC-relative `ldr` |
| **MMIO** | Memory-mapped I/O — hardware registers accessed as memory addresses |
| **SIO** | Single-cycle I/O — the fast GPIO block in the RP2350, at `0xd0000000` |
| **Thumb bit** | Bit 0 of a Cortex-M function pointer; selects Thumb instruction mode |
| **UF2** | USB Flashing Format — the file format the Pico 2 bootloader accepts |
| **Vector table** | The first words of flash: initial stack pointer and exception vectors |

---

**Remember:** the ELF tells you what every address is, and the `.bin` is what you actually patch. Verify the static behavior from the recovered instructions.

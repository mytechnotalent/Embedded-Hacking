# Week 11-IDA: IDA Pro — Hack Structs & Functions with the NEC IR Remote (Raw `.bin`)

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

- Build the two lesson projects with `Release` and get an `.elf` and a raw `.bin` for each
- Dump the **ELF symbol map** with `arm-none-eabi-nm` and use it as ground truth
- Load each raw `.bin` into IDA at `0x10000000`
- **Resolve the functions in the IDA GUI** using the ELF symbol map
- See how the compiler **flattens a C struct into hard-coded immediates** and **inlines every helper function**
- **Patch** the LED pin immediates and the NEC format string, export, convert and export
- Understand the security lesson: **the log says one thing while the hardware does another**

---

## How This Guide Works

Each project builds two files:

| File | What it is | How we use it |
| ---- | ---------- | ------------- |
| `.elf` | The linked image with a full symbol table and DWARF | Ground truth for every function address and signature |
| `.bin` | The raw flash image, no headers, no symbols | The image we load into IDA and reverse |

The `.bin` is built **from** the `.elf`, so the ELF tells you exactly what is at every address. We reverse-engineer the raw `.bin` the way a real extracted firmware image is reversed.

> **Build `Release`, not `Debug`.** Every address in this guide matches the current `Release` builds. `Release` flattens the LED struct into plain immediates and inlines every `static` helper (`make_default_leds`, `init_led_gpios`, `process_ir_key`, `poll_ir`, and in Project 2 `ir_to_led_number`, `get_led_pin`, `leds_all_off`, `blink_led`, `process_ir_led_command`, `handle_ir_key`, `poll_and_handle_ir`) into `main`. If you build `Debug`, the SDK function addresses move and the helpers stay separate, so nothing lines up. Always build `Release` for this lesson.

The workflow is static-only: import the raw image, recover the code, decompile it, and patch the artifact:

d prove what the code does.
ge.
3. Resolve the functions in IDA using the ELF symbol map.
4. Patch the bytes, export, convert and export.

| Project | Prints | Also does | The hacks |
| ------- | ------ | --------- | --------- |
| `0x0023_structures` | `IR receiver on GPIO 5 ready`, then `NEC command: 0xNN` per key | lights LED1/2/3 on GPIO 16/17/18 from the flattened struct | move LED1 to GPIO 18 live; swap LED1↔LED3 pins; rename the `NEC` string |
| `0x0026_functions` | the same, plus `LED N activated on GPIO P` | blinks the mapped LED 3× then holds it | forge the decoded key live; swap LED1↔LED3 pins (log desync); rename the `NEC` string |

> **The struct disappears.** `simple_led_ctrl_t` has six members (three `uint8_t` pins and three `bool` states), but the optimizer proves it never escapes `main`, so it is never placed in memory. `leds.led1_pin` becomes the literal `16`, `leds.led2_pin` becomes `17`, `leds.led3_pin` becomes `18`, and the `bool` states become register values. That is why you patch **immediates**, not a struct field.

> **The functions disappear too.** Every `static` helper is inlined, so there is no `process_ir_key` or `blink_led` symbol to rename. You see their bodies directly inside `main`. The only real functions `main` calls are the SDK routines and `ir_init`/`ir_getkey`.

> **The SVD file lives in `WEEK04`.** If you want the RP2350 peripheral register map for the SIO/GPIO side of the lab, it is `Embedded-Hacking/WEEK04/rp2350.svd` — it is **not** in `WEEK11`.

### Background: the NEC IR remote in one paragraph

An IR receiver on **GPIO 5** demodulates a 38 kHz carrier and presents the NEC frame as a digital mark/space train. `ir_getkey()` waits for the 9 ms leader + 4.5 ms space, samples 32 bits by timing the marks, then validates that the address and command pairs are bitwise inverses. It returns the **command byte** (`0x0C`, `0x18`, or `0x5E` for buttons 1, 2, 3) or `-1`. `main` maps that byte to one of three LEDs on **GPIO 16 (red), 17 (green), 18 (yellow)**. Because the struct is flattened, that mapping is a set of hard-coded pin numbers in the loop — exactly what we patch.

---

## Part 1: Build and Get the Symbol Map

### Step 1: Install the toolchain

**Windows x64**

- Install the **Raspberry Pi Pico** extension in VS Code. It installs the ARM GNU toolchain, CMake, Ninja, and the Pico SDK.
- Install **IDA Pro** and complete its license activation.
- Install **PuTTY** for the serial monitor.

**macOS Apple Silicon**


All must report `arm64`. If any is `x86_64`, put the Apple Silicon prefix first for the session and check again:

```bash
export PATH="/opt/homebrew/bin:$PATH"
hash -r
file "$(which cmake)"
```


**Windows x64** and **Linux x64** do not have this problem. Skip to Step 3.

### Step 3: Build the two projects with `Release`

Run this once inside `0x0023_structures/` and once inside `0x0026_functions/`:

```bash
cmake -B build -G Ninja -DPICO_BOARD=pico2 -DPICO_PLATFORM=rp2350 -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Point IDA at this repository (once).** Every console snippet below reads the repo root from `~/.embedded-hacking-repo`, so IDA never needs a database open and nothing is hardcoded. From the repo root, run once:

**macOS / Linux:**

```bash
pwd > ~/.embedded-hacking-repo
```

**Windows (PowerShell):**

```powershell
(Get-Location).Path | Set-Content "$env:USERPROFILE\.embedded-hacking-repo"
```

**Then build from the IDA console**, so the whole build -> patch -> flash loop stays inside IDA. The console inherits a minimal `PATH` — on macOS just `/usr/bin:/bin:/usr/sbin:/sbin` — so it does not see Homebrew; add your package manager's `bin` first, then run plain `cmake`.

**macOS Apple Silicon:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
os.environ["PATH"] = "/opt/homebrew/bin:" + os.environ["PATH"]  # the console's PATH omits Homebrew
for name in ("0x0023_structures", "0x0026_functions"):
    proj = os.path.join(root, name)
    subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                    "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
    subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

**Linux x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
for name in ("0x0023_structures", "0x0026_functions"):
    proj = os.path.join(root, name)
    subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                    "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
    subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

**Windows x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
for name in ("0x0023_structures", "0x0026_functions"):
    proj = os.path.join(root, name)
    subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                    "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
    subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

Each build directory now contains the pair we need:

- `0x0023_structures/build/0x0023_structures.elf` and `.bin` — `.bin` is **16372** bytes (`0x3ff4`)
- `0x0026_functions/build/0x0026_functions.elf` and `.bin` — `.bin` is **16476** bytes (`0x405c`)

If the ARM toolchain is not on your `PATH`, add `-DPICO_TOOLCHAIN_PATH=...`:

| OS | Typical toolchain path |
| -- | ---------------------- |
| Windows x64 | `C:/Program Files/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin` |
| macOS Apple Silicon | `/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin` |
| Linux x64 | `/usr` |

### Step 4: Dump the ELF symbol map

This is the ground truth for the whole lesson. Run `arm-none-eabi-nm` on each ELF and keep the output in a terminal or a text file:

**macOS Apple Silicon / Linux x64:**

```bash
arm-none-eabi-nm -n --defined-only build/0x0023_structures.elf | grep -E ' [Tt] '
arm-none-eabi-nm -n --defined-only build/0x0026_functions.elf | grep -E ' [Tt] '
```

**Windows x64:**

```powershell
arm-none-eabi-nm -n --defined-only build\0x0023_structures.elf | Select-String ' [Tt] '
arm-none-eabi-nm -n --defined-only build\0x0026_functions.elf | Select-String ' [Tt] '
```

Each line is `address type name`. The `T`/`t` type is a function. The signatures below come from the ELF's DWARF debug info queried with `arm-none-eabi-gdb -batch -ex "ptype <name>"`, so they are exact.

**Project 1 — `0x0023_structures` — our code and the startup chain:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` | reset entry |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` | calls `runtime_init`, `main`, `exit` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` | copies `.data` from flash to SRAM |
| `0x100001e4` | `_init` | `void _init(void)` | runs `.init_array` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` | C runtime boilerplate |
| `0x10000234` | `main` | `int main(void)` | the lesson function (struct flattened, all helpers inlined) |
| `0x100002cc` | `ir_init` | `void ir_init(uint8_t)` | the `ir.c` receiver init |
| `0x100002f4` | `ir_getkey` | `int ir_getkey(void)` | the blocking NEC decoder (timing helpers inlined) |

**Project 1 — the GPIO, UART, stdio, and `printf` chain `main` reaches:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x100004e8` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` | SDK GPIO function select |
| `0x10000524` | `gpio_set_pulls` | `void gpio_set_pulls(uint, bool, bool)` | SDK pull-up/down (used by `ir_init`) |
| `0x1000054c` | `gpio_init` | `void gpio_init(uint)` | SDK GPIO init |
| `0x10000fa8` | `sleep_ms` | `void sleep_ms(uint32_t)` | SDK millisecond delay |
| `0x1000118c` | `time_us_64` | `uint64_t time_us_64(void)` | SDK microsecond clock (NEC timing) |
| `0x100011a0` | `busy_wait_us` | `void busy_wait_us(uint64_t)` | UART timing loop |
| `0x10001220` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` | SDK UART init |
| `0x100013f4` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` | UART clock lookup |
| `0x1000310c` | `exit` | `void exit(int)` | C runtime exit |
| `0x10003114` | `runtime_init` | `void runtime_init(void)` | SDK runtime init |
| `0x10003140` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` | CRLF output driver |
| `0x10003250` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` | buffered string output |
| `0x1000333c` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` | enable the UART driver |
| `0x10003364` | `stdio_init_all` | `bool stdio_init_all(void)` | SDK serial init |
| `0x100033f4` | `__wrap_puts` | `int __wrap_puts(const char*)` | the `puts` wrapper (adjacent stdio family) |
| `0x10003430` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` | printf core |
| `0x100034f4` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` | the `printf` wrapper (both prints) |
| `0x100036b0` | `stdio_uart_init` | `void stdio_uart_init(void)` | SDK UART stdio init |
| `0x100037f0` | `strlen` | `size_t strlen(const char*)` | C runtime string length |

**Project 2 — `0x0026_functions` — our code and the startup chain:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` | reset entry |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` | calls `runtime_init`, `main`, `exit` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` | copies `.data` from flash to SRAM |
| `0x100001e4` | `_init` | `void _init(void)` | runs `.init_array` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` | C runtime boilerplate |
| `0x10000234` | `main` | `int main(void)` | the lesson function (struct flattened, all helpers inlined) |
| `0x10000318` | `ir_init` | `void ir_init(uint8_t)` | the `ir.c` receiver init |
| `0x10000340` | `ir_getkey` | `int ir_getkey(void)` | the blocking NEC decoder (timing helpers inlined) |

**Project 2 — the GPIO, UART, stdio, and `printf` chain `main` reaches:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x10000534` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` | SDK GPIO function select |
| `0x10000570` | `gpio_set_pulls` | `void gpio_set_pulls(uint, bool, bool)` | SDK pull-up/down (used by `ir_init`) |
| `0x10000598` | `gpio_init` | `void gpio_init(uint)` | SDK GPIO init |
| `0x10000ff0` | `sleep_ms` | `void sleep_ms(uint32_t)` | SDK millisecond delay |
| `0x100011d4` | `time_us_64` | `uint64_t time_us_64(void)` | SDK microsecond clock (NEC timing) |
| `0x100011e8` | `busy_wait_us` | `void busy_wait_us(uint64_t)` | UART timing loop |
| `0x10001268` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` | SDK UART init |
| `0x1000143c` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` | UART clock lookup |
| `0x10003154` | `exit` | `void exit(int)` | C runtime exit |
| `0x1000315c` | `runtime_init` | `void runtime_init(void)` | SDK runtime init |
| `0x10003188` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` | CRLF output driver |
| `0x10003298` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` | buffered string output |
| `0x10003384` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` | enable the UART driver |
| `0x100033ac` | `stdio_init_all` | `bool stdio_init_all(void)` | SDK serial init |
| `0x1000343c` | `__wrap_puts` | `int __wrap_puts(const char*)` | the `puts` wrapper (adjacent stdio family) |
| `0x10003478` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` | printf core |
| `0x1000353c` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` | the `printf` wrapper (all three prints) |
| `0x100036f8` | `stdio_uart_init` | `void stdio_uart_init(void)` | SDK UART stdio init |
| `0x10003838` | `strlen` | `size_t strlen(const char*)` | C runtime string length |

> **`main` is `0x10000234` in both projects.** Both programs put `main` at the same address because the startup code and the linker layout are identical; only the body of `main` and the functions after it move. In a `Debug` build the helpers stay separate and `main` moves — another reason to build `Release`.

> **`printf` in our source is `__wrap_printf` in the binary.** The SDK links our `printf` calls to its `__wrap_printf` wrapper, which forwards to `__wrap_vprintf`. The `__wrap_puts` symbol exists in the image (the stdio family always does), but our `printf` path does not call it.

## Part 2: Load the Raw `.bin` into IDA

Start from a fresh IDA state. If you already have a database for this lesson, **close it and start over**; a stale database keeps old names and patches.

### Step 7: Bring the raw `.bin` into IDA

A raw `.bin` has no headers, so IDA cannot know where it belongs or what architecture it is. You must supply both. If you just double-click the `.bin`, IDA may load it with a guessed architecture, and every address in this lesson will be wrong.

#### 1.1 Open the file
1. **File ▸ Open…** (Ctrl/Cmd+O).
2. Select the `.bin` file and click **Open**.

IDA opens the **"Load a new file"** dialog because a raw `.bin` has no header.

#### 1.2 Set the format and processor
In the *Load a new file* dialog:
1. **Format**: leave it as **Binary file** (IDA auto-detects a headerless image).
2. **Processor type**: click the **…** button next to the processor field. In the *Processor type* dialog:
   - Family: **ARM**
   - Type: **ARM Little-endian** (the 32-bit ARM module).
   - Click **OK**.

> If IDA asks **"Do you want to disassemble it as 64-bit code?"** — click **No**.
> The RP2350 is 32-bit; answering *Yes* loads AArch64 and you get `X` registers / data. (This is the single most common cause of the `DCB` wall.)

#### 1.3 Configure the ARM architecture and Thumb mode
Still in the *Load a new file* dialog, open the processor options (the **Processor options** / **Set…** button, which opens the dialog titled **ARM architecture options**):
- **ARM architecture**: choose **ARMv8-M** (this is the Cortex-M33 / RP2350 core).
- **Thumb instructions**: select **Thumb-2** (ARMv8-M is Thumb-only, so this is the correct mode).
- Leave **automatic ARM-THUMB switch** off; we want Thumb throughout.

#### 1.4 Set the loading address
- **Loading address**: enter **`10000000`** (hex; the RP2350 XIP flash base).
  IDA rejects addresses that don't look like ROM/RAM with *"The loading address should belong to RAM or ROM"* — `0x10000000` is the correct flash window.

Leave **Manual load** and **Create segments** at their defaults. Click **OK**.

#### 1.5 Force Thumb, then seed the code from the vector table
A raw `.bin` has no header, so IDA's loader has **no entry point** and defaults the segment to **ARM mode** (which cannot decode Thumb). Two required steps — in this order:

**A. Set the segment to Thumb (do this first).**
- **Edit ▸ Segments ▸ Set default segment register value…** → register **`T`** = **`1`** (or press **`Alt+G`**). IDA reanalyzes the segment as Thumb automatically.

**B. Seed the code from the vector table.**
1. **G** → `10000000`. The first two words are the initial stack pointer and the **reset vector**.
2. **Read the 4-byte word at `0x10000004`** (do **not** assume its value — it differs per build).
3. **Clear bit 0** (the ARM **Thumb** flag) → the reset handler address.
4. **G** to that address → press **C** (**MakeCode**). IDA disassembles the reset handler and follows its `bl`s into `main`.

Once `main` shows code, the import is correct.

**Why IDA needs this and others don't:** IDA's generic raw-binary ARM loader treats the image as flat data and waits for you to set Thumb and mark the entry point. It's an IDA quirk, not a mistake on your part.

### Step 8: Save it as a IDA database (`.i64`)

IDA never writes back into the `.bin`. Your names, comments, types, and patches live in a separate **`.i64`** database. Save one now, before you make any changes:

1. Choose `File -> Save As...`.
2. Save it next to the image as `0x0023_structures.i64`.
3. From now on, save with `File -> Save` (`Cmd+S` on macOS, `Ctrl+S` on Windows/Linux) whenever you rename or patch.

| File | Role |
| ---- | ---- |
| `0x0023_structures.bin` | the raw firmware image; IDA never modifies it |
| `0x0023_structures.i64` | your analysis database: names, types, comments, and patches |

When you come back later, **open the `.i64`**, not the `.bin`; that restores all your work. You export the patched image out of this view later, in Step 19.

### Step 9: The views you will use

- **Linear view:** the disassembly listing. You navigate, read, and patch here.
- **Graph view:** the control-flow graph of the current function.
- **Decompiler (HLIL):** the pseudo-C decompilation.
- **Hex view:** raw bytes, used for patching.
- **Function list:** the sidebar list of every detected function.

Navigation: `G` go to address, `N` rename, `Y` set type or signature, `;` add a comment.

> **macOS function keys:** the top-row `F` keys are usually mapped to system functions. Every step here uses menu paths that work without them.

---

## Part 3: Static — Resolve the Functions in IDA and Patch (Project 1)

### Step 16: Resolve the functions in the IDA GUI

We now name the functions in IDA using the ELF symbol map from Step 4. IDA loaded the raw `.bin` with **no symbols**, so every function shows as `sub_<addr>` — resolving means giving each one its real name and signature.

Three keys do all the work:

| Key | IDA action | Use it for |
| --- | --- | --- |
| `G` | Go to address | Jump to a function's address |
| `Y` | **Change Type** | Set the function's signature. The dialog shows the full prototype, so this sets the name *and* the type in one step. |
| `N` | Rename | Rename only, when you just want the name and not the type |

For each function below: `G` to its address, then **`Y` (Change Type)** and type the prototype from the table.

#### How to resolve a function in IDA (`Y`)

`Y` is the **Change Type** key, and it is what actually resolves the function — it turns `void sub_10003364()` into `bool stdio_init_all(void)`. The Change Type dialog shows the full declaration (name and type), so typing the prototype sets both:

1. `G` to the function's address. The cursor lands on the function.
2. Press **`Y`**. In the Change Type dialog, type the prototype from the table exactly — for example `bool stdio_init_all(void)` — and press Enter.

If `Y` seems to do nothing, confirm the cursor is on the function, or right-click it and pick **Change Type...**. IDA parses what you type and silently keeps the old type if it does not parse, so glance at the header after each `Y`.

#### Worked example: `main`

1. Press `G`, type `0x10000234`, press Enter. The cursor lands on `sub_10000234`.
2. Press **`Y`** (Change Type), type `int main(void)`, press Enter.

> **IDA shows `int32_t` where Ghidra shows `int`.** After you set `int main(void)`, the decompiler header may read `int32_t main(void)`. That is the same type — on this platform `int` is 32 bits and IDA's parser normalises it to `int32_t`. Do not fight it; it is not an error.

#### Worked example: `ir_init`

1. `G` -> `0x100002cc`.
2. `Y` -> `void ir_init(uint8_t pin)`.

It takes a `uint8_t` pin; `main` calls it with `5` (`movs r0, #5`). It opens with `gpio_init(pin)`, then writes the direction and calls `gpio_set_pulls` for the pull-up.

#### Worked example: `ir_getkey`

1. `G` -> `0x100002f4`.
2. `Y` -> `int ir_getkey(void)`.

It returns `-1` on timeout and the command byte otherwise. The NEC timing helpers (`wait_for_level`, `wait_leader`, `read_nec_bit`, `read_32_bits`, `validate_nec_frame`) are all `static` and **inlined** into it, so you will not find them as separate functions.

#### Worked example: `gpio_init`

1. `G` -> `0x1000054c`.
2. `Y` -> `void gpio_init(uint gpio)`.

`main` calls it three times with `16`, `17`, `18` — the flattened struct pins.

#### Worked example: `stdio_init_all`

1. `G` -> `0x10003364`.
2. `Y` -> `bool stdio_init_all(void)`.

It returns **`bool`**, not `void` — the ELF says `_Bool stdio_init_all(void)`. Our `main` ignores the return value, so the decompiler still reads cleanly.

#### Worked example: `__wrap_printf`

1. `G` -> `0x100034f4`.
2. `Y` -> `int __wrap_printf(const char *fmt, ...)`. Keep the `...` — `printf` is variadic.

> **`printf` in our source is `__wrap_printf` in the binary.** The SDK links our `printf` calls to its `__wrap_printf` wrapper.

The rest of the chain is the same two keystrokes per function (`G`, then `Y`). This is **our code plus the library functions it actually calls** — not the whole SDK. The call chain for this project:

```
main
├── stdio_init_all ── stdio_uart_init ── gpio_set_function, uart_init
│   │                                   └── uart_init ── clock_get_hz, busy_wait_us
│   ├── stdio_set_driver_enabled
│   ├── stdio_out_chars_crlf
│   └── stdio_put_string ── strlen, time_us_64
├── gpio_init
├── ir_init ── gpio_init, gpio_set_pulls
├── ir_getkey ── time_us_64   (the NEC timing helpers are inlined)
└── __wrap_printf ── __wrap_vprintf
```

**Project 1 — resolve every function in that chain:**

| Address | Rename to (`N`) | Signature (`Y`) |
| ------- | --------------- | --------------- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` |
| `0x100001e4` | `_init` | `void _init(void)` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` |
| **`0x10000234`** | **`main`** | **`int main(void)`** |
| `0x100002cc` | `ir_init` | `void ir_init(uint8_t)` |
| `0x100002f4` | `ir_getkey` | `int ir_getkey(void)` |
| `0x100004e8` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` |
| `0x10000524` | `gpio_set_pulls` | `void gpio_set_pulls(uint, bool, bool)` |
| `0x1000054c` | `gpio_init` | `void gpio_init(uint)` |
| `0x10000fa8` | `sleep_ms` | `void sleep_ms(uint32_t)` |
| `0x1000118c` | `time_us_64` | `uint64_t time_us_64(void)` |
| `0x100011a0` | `busy_wait_us` | `void busy_wait_us(uint64_t)` |
| `0x10001220` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` |
| `0x100013f4` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` |
| `0x1000310c` | `exit` | `void exit(int)` |
| `0x10003114` | `runtime_init` | `void runtime_init(void)` |
| `0x10003140` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` |
| `0x10003250` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` |
| `0x1000333c` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` |
| `0x10003364` | `stdio_init_all` | `bool stdio_init_all(void)` |
| `0x100033f4` | `__wrap_puts` | `int __wrap_puts(const char*)` |
| `0x10003430` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` |
| `0x100034f4` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` |
| `0x100036b0` | `stdio_uart_init` | `void stdio_uart_init(void)` |
| `0x100037f0` | `strlen` | `size_t strlen(const char*)` |

> **A `void` return type may not stick — here is the fix.** IDA treats `void` as low-confidence, and its analysis can override it with an inferred type — most often `int32_t` on this 32-bit target. It is most visible on `_reset_handler` (a hand-written assembly entry that never returns normally), but it can happen to **any** function whose return type IDA thinks it can infer.
>
> Setting the full signature with `Y` reproduces the unwanted `int32_t`, and `fn.return_type = ...` fails too. What works is the **return-value** setter:
>
> ```python
> from binaryninja import ReturnValue, Type
> fn = bv.get_function_at(0x1000015c)
> if fn is not None:
>     fn.return_value = ReturnValue(Type.void())
> ```
>
> That holds `_reset_handler` at `void` even after reanalysis. If it still will not stick, leave it — it does not affect the rest of the lesson.

> **Shortcut — resolves name *and* type for every function.** Instead of doing `N` + `Y` by hand, paste this into IDA's Python console (`Plugins -> Python Console`). It sets each function's name and signature programmatically:
>
> ```python
> from binaryninja import Symbol, SymbolType
> # The raw .bin has no headers, so these SDK types don't exist. set_user_type()
> # re-parses each signature as C, so an undefined name raises
> # "SyntaxError: unknown type name '...'". Define them first.
> sdk = bv.parse_types_from_string("""
> typedef unsigned int uint;
> typedef char* va_list;
> typedef unsigned long clock_handle_t;
> struct stdio_driver;
> typedef struct stdio_driver stdio_driver_t;
> struct uart_inst;
> typedef struct uart_inst uart_inst_t;
> enum gpio_function {
>     GPIO_FUNC_XIP = 0, GPIO_FUNC_SPI = 1, GPIO_FUNC_UART = 2, GPIO_FUNC_I2C = 3,
>     GPIO_FUNC_PWM = 4, GPIO_FUNC_SIO = 5, GPIO_FUNC_PIO0 = 6, GPIO_FUNC_PIO1 = 7,
>     GPIO_FUNC_GPCK = 8, GPIO_FUNC_USB = 9, GPIO_FUNC_NULL = 0x1f,
> };
> typedef enum gpio_function gpio_function_t;
> """)
> for name, ty in sdk.types.items():
>     bv.define_user_type(name, ty)
>
> # address: (name, signature)
> funcs = {
>     0x1000015c: ("_reset_handler",            "void _reset_handler(void)"),
>     0x10000186: ("platform_entry",            "void platform_entry(void)"),
>     0x1000019a: ("data_cpy",                  "void data_cpy(void*, void*, void*)"),
>     0x100001e4: ("_init",                     "void _init(void)"),
>     0x10000210: ("frame_dummy",               "void frame_dummy(void)"),
>     0x10000234: ("main",                      "int main(void)"),
>     0x100002cc: ("ir_init",                   "void ir_init(uint8_t)"),
>     0x100002f4: ("ir_getkey",                 "int ir_getkey(void)"),
>     0x100004e8: ("gpio_set_function",         "void gpio_set_function(uint, gpio_function_t)"),
>     0x10000524: ("gpio_set_pulls",            "void gpio_set_pulls(uint, bool, bool)"),
>     0x1000054c: ("gpio_init",                 "void gpio_init(uint)"),
>     0x10000fa8: ("sleep_ms",                  "void sleep_ms(uint32_t)"),
>     0x1000118c: ("time_us_64",                "uint64_t time_us_64(void)"),
>     0x100011a0: ("busy_wait_us",              "void busy_wait_us(uint64_t)"),
>     0x10001220: ("uart_init",                 "uint uart_init(uart_inst_t*, uint)"),
>     0x100013f4: ("clock_get_hz",              "unsigned long clock_get_hz(clock_handle_t)"),
>     0x1000310c: ("exit",                      "void exit(int)"),
>     0x10003114: ("runtime_init",              "void runtime_init(void)"),
>     0x10003140: ("stdio_out_chars_crlf",      "void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)"),
>     0x10003250: ("stdio_put_string",          "int stdio_put_string(const char*, int, bool, bool)"),
>     0x1000333c: ("stdio_set_driver_enabled",  "void stdio_set_driver_enabled(stdio_driver_t*, bool)"),
>     0x10003364: ("stdio_init_all",            "bool stdio_init_all(void)"),
>     0x100033f4: ("__wrap_puts",               "int __wrap_puts(const char*)"),
>     0x10003430: ("__wrap_vprintf",            "int __wrap_vprintf(const char*, va_list)"),
>     0x100034f4: ("__wrap_printf",             "int __wrap_printf(const char*, ...)"),
>     0x100036b0: ("stdio_uart_init",           "void stdio_uart_init(void)"),
>     0x100037f0: ("strlen",                    "size_t strlen(const char*)"),
> }
> for addr, (name, sig) in funcs.items():
>     bv.define_user_symbol(Symbol(SymbolType.FunctionSymbol, addr, name))
>     f = bv.get_function_at(addr)
>     if f is not None:
>         f.set_user_type(sig)
> ```
>
> SDK type names (`stdio_driver_t`, `uart_inst_t`, `gpio_function_t`, `clock_handle_t`, plus `uint`, `va_list`) are **not** in the raw `.bin`. `set_user_type` re-parses each signature as C, so an undefined name raises `SyntaxError: unknown type name '...'` and stops the loop — it is not harmless. The `sdk` block above defines them first. Standard names (`uint8_t`, `uint32_t`, `uint64_t`, `bool`, `size_t`) are built in.

### Step 17: Read `main` in the decompiler

Open the **Decompiler** view on `main`. The whole program is one function because every `static` helper was inlined:

```asm
10000234 <main>:
10000234:	b508      	push	{r3, lr}
10000236:	f003 f895 	bl	10003364 <stdio_init_all>
1000023a:	2010      	movs	r0, #16
1000023c:	f000 f986 	bl	1000054c <gpio_init>
10000240:	2510      	movs	r5, #16
10000242:	f04f 0401 	mov.w	r4, #1
10000246:	ec44 5044 	mcrr	0, 4, r5, r4, cr4
1000024a:	2011      	movs	r0, #17
1000024c:	f000 f97e 	bl	1000054c <gpio_init>
10000250:	2311      	movs	r3, #17
10000252:	ec44 3044 	mcrr	0, 4, r3, r4, cr4
10000256:	2012      	movs	r0, #18
10000258:	f000 f978 	bl	1000054c <gpio_init>
1000025c:	2312      	movs	r3, #18
1000025e:	ec44 3044 	mcrr	0, 4, r3, r4, cr4
10000262:	2005      	movs	r0, #5
10000264:	f000 f832 	bl	100002cc <ir_init>
10000268:	2105      	movs	r1, #5
1000026a:	4816      	ldr	r0, [pc, #88]
1000026c:	f003 f942 	bl	100034f4 <__wrap_printf>
10000270:	f000 f840 	bl	100002f4 <ir_getkey>
10000274:	1e04      	subs	r4, r0, #0
10000276:	db21      	blt.n	100002bc <main+0x88>
10000278:	4621      	mov	r1, r4
1000027a:	4813      	ldr	r0, [pc, #76]
1000027c:	f003 f93a 	bl	100034f4 <__wrap_printf>
10000280:	f1a4 030c 	sub.w	r3, r4, #12
10000284:	fab3 f383 	clz	r3, r3
10000288:	095b      	lsrs	r3, r3, #5
1000028a:	ec43 5040 	mcrr	0, 4, r5, r3, cr0
1000028e:	f1a4 0218 	sub.w	r2, r4, #24
10000292:	fab2 f282 	clz	r2, r2
10000296:	2311      	movs	r3, #17
10000298:	0952      	lsrs	r2, r2, #5
1000029a:	ec42 3040 	mcrr	0, 4, r3, r2, cr0
1000029e:	f1a4 045e 	sub.w	r4, r4, #94
100002a2:	fab4 f484 	clz	r4, r4
100002a6:	2312      	movs	r3, #18
100002a8:	0964      	lsrs	r4, r4, #5
100002aa:	ec44 3040 	mcrr	0, 4, r3, r4, cr0
100002ae:	200a      	movs	r0, #10
100002b0:	f000 fe7a 	bl	10000fa8 <sleep_ms>
100002b4:	f000 f81e 	bl	100002f4 <ir_getkey>
100002b8:	1e04      	subs	r4, r0, #0
100002ba:	dadd      	bge.n	10000278 <main+0x44>
100002bc:	2001      	movs	r0, #1
100002be:	f000 fe73 	bl	10000fa8 <sleep_ms>
100002c2:	e7d5      	b.n	10000270 <main+0x3c>
100002c4:	100038b0 	.word	0x100038b0
100002c8:	100038d0 	.word	0x100038d0
```

The decompiler reads roughly:

```c
int32_t main(void)
{
    stdio_init_all();
    gpio_init(0x10); gpio_set_dir(0x10, 1);   // led1_pin = 16
    gpio_init(0x11); gpio_set_dir(0x11, 1);   // led2_pin = 17
    gpio_init(0x12); gpio_set_dir(0x12, 1);   // led3_pin = 18
    ir_init(5);
    __wrap_printf("IR receiver on GPIO %d ready\n", 5);
    do
    {
        int32_t key = ir_getkey();
        if (key >= 0)
        {
            __wrap_printf("NEC command: 0x%02X\n", key);
            mcrr(0x10, key == 0x0c);   // led1_pin = 16
            mcrr(0x11, key == 0x18);   // led2_pin = 17
            mcrr(0x12, key == 0x5e);   // led3_pin = 18
            sleep_ms(10);
        }
        else
        {
            sleep_ms(1);
        }
    } while (true);
}
```

- **There is no struct.** `16`, `17`, `18` are immediates; `led1_state` etc. are the `clz`-computed register values.
- The `clz`/`lsrs` pair is how the compiler turns `(key == 0x0C)` into a `0`/`1` without a branch: `sub.w r3, r4, #12` sets the flags, `clz r3, r3` counts leading zeros, `lsrs r3, r3, #5` reduces it to `0` or `1`.
- `0x100002c4` and `0x100002c8` point at `"IR receiver on GPIO %d ready\n"` (`0x100038b0`) and `"NEC command: 0x%02X\n"` (`0x100038d0`) in `.rodata`.

### Step 18: Patch 1 — swap LED1 and LED3 pins

The original lesson swaps LED pin assignments. LED1 is the red LED on GPIO 16 and LED3 is the yellow LED on GPIO 18. Swap them so button **"1"** lights yellow and button **"3"** lights red. The two pins are hard-coded immediates:

| Address | Instruction | Bytes before | Bytes after | Role |
| ------- | ----------- | ------------ | ----------- | ---- |
| `0x10000240` | `movs r5, #16` | `10 25` | `12 25` | LED1's pin (`r5`) 16 -> 18 |
| `0x100002a6` | `movs r3, #18` | `12 23` | `10 23` | LED3's pin (`r3`) 18 -> 16 |

In the **Hex** view (`View -> Hex`, lock off) change the low byte of each, then right-click `main` -> `Reanalyze`. Or in the Python console:

```python
bv.write(0x10000240, b"\x12")  # movs r5, #18  (LED1 -> GPIO 18)
bv.write(0x100002a6, b"\x10")  # movs r3, #16  (LED3 -> GPIO 16)
print(bv.read(0x10000240, 2).hex(" "))  # -> 12 25
print(bv.read(0x100002a6, 2).hex(" "))  # -> 10 23
```

After reanalysis the loop reads `movs r5, #18` and `movs r3, #16`, so button **"1"** drives GPIO 18 (yellow) and button **"3"** drives GPIO 16 (red). The `NEC command:` log still prints the *command byte*, so the log and the physical LED mapping no longer agree — the log desynchronization.

### Step 18b: Patch 2 — rename the `NEC` string to `PWN`

The format string `"NEC command: 0x%02X\n"` starts at `0x100038d0`. Its first three bytes are `4e 45 43` (`NEC`). Change them to `50 57 4e` (`PWN`), leaving the ` command: 0x%02X\n` tail untouched, so the line prints `PWN command: 0x0C`.

**Option A — Hex view:** go to `0x100038d0` and change the three bytes `4e 45 43` to `50 57 4e`, then reanalyze.

**Option B — Python console:**

```python
bv.write(0x100038d0, b"PWN")
print(bv.read(0x100038d0, 20))  # -> b'PWN command: 0x%02X\n\x00'
```

Keep the replacement exactly three bytes. If you use a shorter string you must pad it, or `%02X` shifts and `printf` reads the wrong argument. A longer string would overwrite the ` command:` tail.

### Step 19: Export the patched `.bin`

```python
import os
seg = next(s for s in bv.segments if s.data_length)  # the loadable image segment
data = bv.read(seg.start, seg.data_length)  # base + size come from the view itself
out = os.path.join(os.path.join(root, "0x0023_structures", "build"), "0x0023_structures-h.bin")
open(out, "wb").write(data)
print(len(data), out)  # -> 16372 /.../build/0x0023_structures-h.bin
```

Where the two numbers come from — nothing is hardcoded:

- **`seg.start`** is the image base IDA loaded the `.bin` at (`0x10000000`), the same value you pass to `uf2conv --base`.
- **`seg.data_length`** is the segment's size in the file (`0x3ff4` = 16372). Exactly one segment carries data (the image); every peripheral and synthetic segment has `data_length == 0`, so `next(...)` picks the image.

> **No relative path.** IDA's Python console runs with a read-only working directory (inside the app bundle), so a relative `open(...)` fails with `OSError: [Errno 30] Read-only file system`. `root` (from `~/.embedded-hacking-repo`, Step 3) is the repo, so the file is written into the project's `build/`.

### Step 20: Convert to UF2

Run from the project directory:

**macOS Apple Silicon / Linux x64:**

```bash
python3 ../uf2conv.py 0x0023_structures-h.bin \
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

**Windows x64:**

```cmd
python ..\uf2conv.py 0x0023_structures-h.bin ^
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

> **Or convert from the IDA console** — `chdir` to a writable directory first (the default one is read-only), then run the script:
>
> ```python
> import os, sys, runpy
> os.chdir(os.path.join(root, "0x0023_structures", "build"))  # the project build dir (writable)
> sys.argv = ["uf2conv.py", "0x0023_structures-h.bin",
>             "--base", "0x10000000", "--family", "0xe48bff59", "--output", "hacked.uf2"]
> runpy.run_path("../../uf2conv.py", run_name="__main__")   # path to your uf2conv.py
> ```

## Part 5: Static — Resolve the Functions and Patch (Project 2)

### Step 26: Resolve the functions in the IDA GUI

Same two keys as Step 16 — `G` to the address, then `Y` (Change Type) to set the prototype — using the Project 2 ELF symbol map from Step 4.

#### Worked example: `main`

1. `G` -> `0x10000234`.
2. `Y` -> `int main(void)` (IDA shows `int32_t main(void)` — the same 32-bit `int`).

#### Worked example: `ir_init`

1. `G` -> `0x10000318`.
2. `Y` -> `void ir_init(uint8_t pin)`.

#### Worked example: `ir_getkey`

1. `G` -> `0x10000340`.
2. `Y` -> `int ir_getkey(void)`.

#### Worked example: `gpio_init`

1. `G` -> `0x10000598`.
2. `Y` -> `void gpio_init(uint gpio)`.

`main` calls it three times with `16`, `17`, `18` — the flattened struct pins.

#### Worked example: `__wrap_printf`

1. `G` -> `0x1000353c`.
2. `Y` -> `int __wrap_printf(const char *fmt, ...)`. Keep the `...` — `printf` is variadic. It forwards to `__wrap_vprintf`.

#### Worked example: `sleep_ms`

1. `G` -> `0x10000ff0`.
2. `Y` -> `void sleep_ms(uint32_t ms)`.

The blink loop and the idle path both load `10` or `50` immediately before calling it.

The call chain for this project is the same as Project 1, plus the extra `printf` and the blink loop:

```
main
├── stdio_init_all ── stdio_uart_init ── gpio_set_function, uart_init
│   │                                   └── uart_init ── clock_get_hz, busy_wait_us
│   ├── stdio_set_driver_enabled
│   ├── stdio_out_chars_crlf
│   └── stdio_put_string ── strlen, time_us_64
├── gpio_init
├── ir_init ── gpio_init, gpio_set_pulls
├── ir_getkey ── time_us_64   (the NEC timing helpers are inlined)
├── __wrap_printf ── __wrap_vprintf   (NEC line and LED line)
└── sleep_ms
```

**Project 2 — resolve every function in that chain:**

| Address | Rename to (`N`) | Signature (`Y`) |
| ------- | --------------- | --------------- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` |
| `0x100001e4` | `_init` | `void _init(void)` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` |
| **`0x10000234`** | **`main`** | **`int main(void)`** |
| `0x10000318` | `ir_init` | `void ir_init(uint8_t)` |
| `0x10000340` | `ir_getkey` | `int ir_getkey(void)` |
| `0x10000534` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` |
| `0x10000570` | `gpio_set_pulls` | `void gpio_set_pulls(uint, bool, bool)` |
| `0x10000598` | `gpio_init` | `void gpio_init(uint)` |
| `0x10000ff0` | `sleep_ms` | `void sleep_ms(uint32_t)` |
| `0x100011d4` | `time_us_64` | `uint64_t time_us_64(void)` |
| `0x100011e8` | `busy_wait_us` | `void busy_wait_us(uint64_t)` |
| `0x10001268` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` |
| `0x1000143c` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` |
| `0x10003154` | `exit` | `void exit(int)` |
| `0x1000315c` | `runtime_init` | `void runtime_init(void)` |
| `0x10003188` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` |
| `0x10003298` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` |
| `0x10003384` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` |
| `0x100033ac` | `stdio_init_all` | `bool stdio_init_all(void)` |
| `0x1000343c` | `__wrap_puts` | `int __wrap_puts(const char*)` |
| `0x10003478` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` |
| `0x1000353c` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` |
| `0x100036f8` | `stdio_uart_init` | `void stdio_uart_init(void)` |
| `0x10003838` | `strlen` | `size_t strlen(const char*)` |

> **Shortcut — resolves name *and* type for every function.** Paste this into IDA's Python console:
>
> ```python
> from binaryninja import Symbol, SymbolType
> # The raw .bin has no headers, so these SDK types don't exist. set_user_type()
> # re-parses each signature as C, so an undefined name raises
> # "SyntaxError: unknown type name '...'". Define them first.
> sdk = bv.parse_types_from_string("""
> typedef unsigned int uint;
> typedef char* va_list;
> typedef unsigned long clock_handle_t;
> struct stdio_driver;
> typedef struct stdio_driver stdio_driver_t;
> struct uart_inst;
> typedef struct uart_inst uart_inst_t;
> enum gpio_function {
>     GPIO_FUNC_XIP = 0, GPIO_FUNC_SPI = 1, GPIO_FUNC_UART = 2, GPIO_FUNC_I2C = 3,
>     GPIO_FUNC_PWM = 4, GPIO_FUNC_SIO = 5, GPIO_FUNC_PIO0 = 6, GPIO_FUNC_PIO1 = 7,
>     GPIO_FUNC_GPCK = 8, GPIO_FUNC_USB = 9, GPIO_FUNC_NULL = 0x1f,
> };
> typedef enum gpio_function gpio_function_t;
> """)
> for name, ty in sdk.types.items():
>     bv.define_user_type(name, ty)
>
> # address: (name, signature)
> funcs = {
>     0x1000015c: ("_reset_handler",            "void _reset_handler(void)"),
>     0x10000186: ("platform_entry",            "void platform_entry(void)"),
>     0x1000019a: ("data_cpy",                  "void data_cpy(void*, void*, void*)"),
>     0x100001e4: ("_init",                     "void _init(void)"),
>     0x10000210: ("frame_dummy",               "void frame_dummy(void)"),
>     0x10000234: ("main",                      "int main(void)"),
>     0x10000318: ("ir_init",                   "void ir_init(uint8_t)"),
>     0x10000340: ("ir_getkey",                 "int ir_getkey(void)"),
>     0x10000534: ("gpio_set_function",         "void gpio_set_function(uint, gpio_function_t)"),
>     0x10000570: ("gpio_set_pulls",            "void gpio_set_pulls(uint, bool, bool)"),
>     0x10000598: ("gpio_init",                 "void gpio_init(uint)"),
>     0x10000ff0: ("sleep_ms",                  "void sleep_ms(uint32_t)"),
>     0x100011d4: ("time_us_64",                "uint64_t time_us_64(void)"),
>     0x100011e8: ("busy_wait_us",              "void busy_wait_us(uint64_t)"),
>     0x10001268: ("uart_init",                 "uint uart_init(uart_inst_t*, uint)"),
>     0x1000143c: ("clock_get_hz",              "unsigned long clock_get_hz(clock_handle_t)"),
>     0x10003154: ("exit",                      "void exit(int)"),
>     0x1000315c: ("runtime_init",              "void runtime_init(void)"),
>     0x10003188: ("stdio_out_chars_crlf",      "void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)"),
>     0x10003298: ("stdio_put_string",          "int stdio_put_string(const char*, int, bool, bool)"),
>     0x10003384: ("stdio_set_driver_enabled",  "void stdio_set_driver_enabled(stdio_driver_t*, bool)"),
>     0x100033ac: ("stdio_init_all",            "bool stdio_init_all(void)"),
>     0x1000343c: ("__wrap_puts",               "int __wrap_puts(const char*)"),
>     0x10003478: ("__wrap_vprintf",            "int __wrap_vprintf(const char*, va_list)"),
>     0x1000353c: ("__wrap_printf",             "int __wrap_printf(const char*, ...)"),
>     0x100036f8: ("stdio_uart_init",           "void stdio_uart_init(void)"),
>     0x10003838: ("strlen",                    "size_t strlen(const char*)"),
> }
> for addr, (name, sig) in funcs.items():
>     bv.define_user_symbol(Symbol(SymbolType.FunctionSymbol, addr, name))
>     f = bv.get_function_at(addr)
>     if f is not None:
>         f.set_user_type(sig)
> ```

### Step 27: Patch 1 — swap LED1 and LED3 pins

The original lesson swaps LED 1 and LED 3. LED1 is the red LED on GPIO 16 and LED3 is the yellow LED on GPIO 18. In the loop the pins are `movs r5, #16` at `0x10000284` (LED1) and `movs r2, #18` at `0x10000290` (LED3). Swap the two immediates:

| Address | Instruction | Bytes before | Bytes after | Role |
| ------- | ----------- | ------------ | ----------- | ---- |
| `0x10000284` | `movs r5, #16` | `10 25` | `12 25` | LED1's pin (`r5`) 16 -> 18 |
| `0x10000290` | `movs r2, #18` | `12 22` | `10 22` | LED3's pin (`r2`) 18 -> 16 |

```python
bv.write(0x10000284, b"\x12")  # movs r5, #18  (LED1 -> GPIO 18)
bv.write(0x10000290, b"\x10")  # movs r2, #16  (LED3 -> GPIO 16)
print(bv.read(0x10000284, 2).hex(" "))  # -> 12 25
print(bv.read(0x10000290, 2).hex(" "))  # -> 10 22
```

Now button **"1"** blinks GPIO 18 (yellow) and button **"3"** blinks GPIO 16 (red). The `LED N activated on GPIO P` prints are **not** changed, so they still say `GPIO 16` and `GPIO 18` — **the log no longer matches the hardware.** That mismatch is the security lesson: the operator's console shows the old, expected mapping while the pins do something else.

> **Optional consistency patch.** If you want the print to tell the truth instead, also change `movs r2, #16` at `0x100002f8` to `#18` (and the key-3 print constant `movne r2, #18` at `0x100002ea` to `#16`). For this lesson we leave them alone on purpose, so the desynchronization is visible.
>
> | Address | Instruction | Bytes before | Bytes after | Role |
> | ------- | ----------- | ------------ | ----------- | ---- |
> | `0x100002f8` | `movs r2, #16` | `10 22` | `12 22` | optional: key-1 print now says GPIO 18 |

### Step 27b: Patch 2 — rename the `NEC` string to `PWN`

The format string starts at `0x10003918`; change its first three bytes `4e 45 43` (`NEC`) to `50 57 4e` (`PWN`):

```python
bv.write(0x10003918, b"PWN")
print(bv.read(0x10003918, 20))  # -> b'PWN command: 0x%02X\n\x00'
```

Exactly three bytes, same rule as Project 1: a shorter string must be padded, a longer one overwrites the ` command:` tail.

### Step 28: Export, convert and export

```python
import os
seg = next(s for s in bv.segments if s.data_length)  # the loadable image segment
data = bv.read(seg.start, seg.data_length)  # base + size from the view itself
out = os.path.join(os.path.join(root, "0x0026_functions", "build"), "0x0026_functions-h.bin")
open(out, "wb").write(data)
print(len(data), out)  # -> 16476 /.../build/0x0026_functions-h.bin
```

`seg.data_length` is the image size (`0x405c` = 16476) read from the view — nothing hardcoded.

**macOS Apple Silicon / Linux x64:**

```bash
python3 ../uf2conv.py 0x0026_functions-h.bin \
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

**Windows x64:**

```cmd
python ..\uf2conv.py 0x0026_functions-h.bin ^
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

Or run the conversion from the IDA console, exactly as in Step 20 (`os.chdir` to the build dir, then `runpy.run_path("../../uf2conv.py", run_name="__main__")` with `sys.argv` set to the arguments above).

Hold **BOOTSEL**, plug in the Pico 2, drag `hacked.uf2` onto the **`RP2350`** drive. Or flash the `.bin` over the Debug Probe with SWD — no BOOTSEL — from the console, exactly as in Step 21.

## Glossary

| Term | Definition |
| ---- | ---------- |
| **`.bss`** | Section for uninitialized global variables; zeroed by startup code |
| **`.data`** | Section for initialized global variables; copied from flash to SRAM at boot |
| **`.elf`** | Linked image with the symbol table; the ground truth for addresses and names |
| **Flattening** | The optimizer replacing struct member accesses with the member's constant value; why `simple_led_ctrl_t` disappears |
| **GPIO** | General Purpose Input/Output — controllable pins on the microcontroller |
ly flash |
| **Inlining** | The optimizer replacing a function call with the function body; why every `static` helper disappears from `main` |
| **Literal pool** | A block of 32-bit constants that Thumb-2 code reaches with PC-relative `ldr` |
| **`mcrr`** | Move to coprocessor from two registers — how the SIO GPIO writes are encoded |
| **NEC** | A common IR protocol: 9 ms leader + 4.5 ms space, then 32 data bits (address, ~address, command, ~command) |
| **`.rodata`** | Read-only section for constants and string literals; stays in flash |
| **SIO** | Single-cycle I/O — the fast GPIO block in the RP2350, at `0xd0000000` |
| **Thumb bit** | Bit 0 of a Cortex-M function pointer; selects Thumb instruction mode |
| **UF2** | USB Flashing Format — the file format the Pico 2 bootloader accepts |
| **Vector table** | The first words of flash: initial stack pointer and exception vectors |

---

**Remember:** the ELF tells you what every address is, and the `.bin` is what you actually patch. Verify the static behavior from the recovered instructions.
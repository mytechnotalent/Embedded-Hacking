# Week 10-IDA: IDA Pro — Hack Static & Dynamic Conditionals with the SG90 Servo (Raw `.bin`)

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

- Build both lesson projects with `Release` and get an `.elf` and a raw `.bin` for each
- Dump the **ELF symbol map** with `arm-none-eabi-nm` and use it as ground truth
- Load each raw `.bin` into IDA at `0x10000000`
- **Resolve the functions in the IDA GUI** using the ELF symbol map
- **Patch** the bytes that control behavior — strings, an IEEE-754 float, an immediate delay, and two `beq` targets — export, convert and export
- Understand how a **static** conditional is optimized away while a **dynamic** conditional must keep its `cmp`/`beq`/`bne` branches

---

## How This Guide Works

Each project builds two files:

| File | What it is | How we use it |
| ---- | ---------- | ------------- |
| `.elf` | The linked image with a full symbol table and DWARF | Ground truth for every function address and signature |
| `.bin` | The raw flash image, no headers, no symbols | The image we load into IDA and reverse |

The `.bin` is built **from** the `.elf`, so the ELF tells you exactly what is at every address. We reverse-engineer the raw `.bin` the way a real extracted firmware image is reversed.

> **Build `Release`, not `Debug`.** Every address in this guide matches the current `Release` builds. `Release` folds the `static` helpers (`print_if_else`, `print_switch`, `sweep_servo`, `eval_if_else`, `process_servo_command`) into `main` and keeps the code layout stable. If you build `Debug`, the SDK function addresses move and the helpers stay separate, so nothing lines up. Always build `Release` for this lesson.

The workflow is static-only: import the raw image, recover the code, decompile it, and patch the artifact:

d prove what the code does.
ge.
3. Resolve the functions in IDA using the ELF symbol map.
4. Patch the bytes, export, convert and export.

| Project | Prints | Also does | The hack |
| ------- | ------ | --------- | -------- |
| `0x001d_static-conditionals` | `1`, then `one`, forever | sweeps the SG90 servo 0° → 180° | angle `180` → `30`, delay `500` → `100`, string `1` → `2`, `one` → `fun` |
| `0x0020_dynamic-conditionals` | `1`+`one` or `2`+`two` from the keyboard | sweeps the servo on `1` / `2` | keys `1`/`2` → `x`/`y`, angle `180` → `30`, skip the prints to go stealth |

> **Two conditionals, two fates.** In Project 1 the `choice` value is hard-coded (`int choice = 1;`), so the compiler proves the condition at compile time and **deletes the `cmp` and the dead branches** — that is a *static* conditional. In Project 2 `choice = getchar()`, so the value is only known at run time and the compiler must emit the `cmp`/`beq`/`bne` chain — that is a *dynamic* conditional. You will see both in the disassembly.

> **Addresses come from your build.** Every address here is from the `Release` build produced in Step 3 and was verified against the current `.elf` files with `arm-none-eabi-nm` and `arm-none-eabi-objdump`. Confirm against your own `.elf` with the command in Step 4.

> **The SVD file lives in `WEEK04`.** If you want the RP2350 peripheral register map for the PWM/UART side of the lab, it is `Embedded-Hacking/WEEK04/rp2350.svd` — it is **not** in `WEEK10`.

### Background: PWM and the SG90 in one paragraph

A servo wants a **50 Hz** signal (a 20 ms frame). The RP2350 system clock is **150 MHz**; the `servo.c` driver divides that down to a **1 MHz** tick (1 tick = 1 µs) and wraps the counter at **20,000**, giving a 20 ms frame. The pulse width picks the angle: **1000 µs = 0°**, **1500 µs = 90°**, **2000 µs = 180°**. `servo_set_angle(float)` clamps the angle, maps it to a pulse in `[1000, 2000]`, and writes the PWM compare level. The float travels in a **general-purpose register (`r0`)**, not `s0` — you will see `vmov s14, r0` at the top of `servo_set_angle`. That is why the live angle hack edits `r0`.

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

Run this once inside `0x001d_static-conditionals/` and once inside `0x0020_dynamic-conditionals/`:

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

**Then build from the IDA console**, so the whole build → patch → flash loop stays inside IDA. The console inherits a minimal `PATH` — on macOS just `/usr/bin:/bin:/usr/sbin:/sbin` — so it does not see Homebrew; add your package manager's `bin` first, then run plain `cmake`.

**macOS Apple Silicon:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
os.environ["PATH"] = "/opt/homebrew/bin:" + os.environ["PATH"]  # the console's PATH omits Homebrew
for name in ("0x001d_static-conditionals", "0x0020_dynamic-conditionals"):
    proj = os.path.join(root, name)
    subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                    "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
    subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

**Linux x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
for name in ("0x001d_static-conditionals", "0x0020_dynamic-conditionals"):
    proj = os.path.join(root, name)
    subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                    "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
    subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

**Windows x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
for name in ("0x001d_static-conditionals", "0x0020_dynamic-conditionals"):
    proj = os.path.join(root, name)
    subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                    "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
    subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

Each build directory now contains the pair we need:

- `0x001d_static-conditionals/build/0x001d_static-conditionals.elf` and `.bin` — `.bin` is **8084** bytes (`0x1f94`)
- `0x0020_dynamic-conditionals/build/0x0020_dynamic-conditionals.elf` and `.bin` — `.bin` is **16188** bytes (`0x3f3c`)

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
arm-none-eabi-nm -n --defined-only build/0x001d_static-conditionals.elf | grep -E ' [Tt] '
arm-none-eabi-nm -n --defined-only build/0x0020_dynamic-conditionals.elf | grep -E ' [Tt] '
```

**Windows x64:**

```powershell
arm-none-eabi-nm -n --defined-only build\0x001d_static-conditionals.elf | Select-String ' [Tt] '
arm-none-eabi-nm -n --defined-only build\0x0020_dynamic-conditionals.elf | Select-String ' [Tt] '
```

Each line is `address type name`. The `T`/`t` type is a function. The signatures below come from the ELF's DWARF debug info queried with `arm-none-eabi-gdb -batch -ex "ptype <name>"`, so they are exact.

**Project 1 — our code and the startup chain:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` | reset entry |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` | calls `runtime_init`, `main`, `exit` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` | copies `.data` from flash to SRAM |
| `0x100001e4` | `_init` | `void _init(void)` | runs `.init_array` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` | C runtime boilerplate |
| `0x10000234` | `main` | `int main(void)` | the lesson function (all three `static` helpers inlined) |
| `0x1000027c` | `servo_init` | `void servo_init(uint8_t)` | the `servo.c` init function |
| `0x10000318` | `servo_set_angle` | `void servo_set_angle(float)` | the `servo.c` angle setter (clamp + PWM level, all helpers inlined) |

**Project 1 — the GPIO, timer, stdio, and `puts` chain `main` reaches:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x100003d8` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` | SDK GPIO function select |
| `0x10000e28` | `sleep_ms` | `void sleep_ms(uint32_t)` | SDK millisecond delay |
| `0x1000100c` | `time_us_64` | `uint64_t time_us_64(void)` | SDK microsecond clock |
| `0x10001020` | `busy_wait_us` | `void busy_wait_us(uint64_t)` | UART timing loop |
| `0x100010a0` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` | SDK UART init |
| `0x10001274` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` | UART/servo clock lookup |
| `0x10001604` | `exit` | `void exit(int)` | C runtime exit |
| `0x1000160c` | `runtime_init` | `void runtime_init(void)` | SDK runtime init |
| `0x10001638` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` | CRLF output driver |
| `0x100016e8` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` | buffered string output |
| `0x100017d4` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` | enable the UART driver |
| `0x100017fc` | `stdio_init_all` | `bool stdio_init_all(void)` | SDK serial init |
| `0x1000188c` | `__wrap_puts` | `int __wrap_puts(const char*)` | the `puts` wrapper (both prints) |
| `0x10001a68` | `stdio_uart_init` | `void stdio_uart_init(void)` | SDK UART stdio init |
| `0x10001ba8` | `strlen` | `size_t strlen(const char*)` | C runtime string length |

**Project 2 — our code and the startup chain:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` | reset entry |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` | calls `runtime_init`, `main`, `exit` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` | copies `.data` from flash to SRAM |
| `0x100001e4` | `_init` | `void _init(void)` | runs `.init_array` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` | C runtime boilerplate |
| `0x10000234` | `main` | `int main(void)` | the lesson function (both `static` helpers inlined) |
| `0x100002dc` | `servo_init` | `void servo_init(uint8_t)` | the `servo.c` init function |
| `0x10000378` | `servo_set_angle` | `void servo_set_angle(float)` | the `servo.c` angle setter (all helpers inlined) |

**Project 2 — the GPIO, timer, stdio, `printf`, `puts`, and `getchar` chain `main` reaches:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x10000438` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` | SDK GPIO function select |
| `0x10000e88` | `sleep_ms` | `void sleep_ms(uint32_t)` | SDK millisecond delay |
| `0x1000106c` | `time_us_64` | `uint64_t time_us_64(void)` | SDK microsecond clock |
| `0x10001080` | `busy_wait_us` | `void busy_wait_us(uint64_t)` | UART timing loop |
| `0x10001100` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` | SDK UART init |
| `0x100012d4` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` | UART/servo clock lookup |
| `0x10002f90` | `vfctprintf` | `int vfctprintf(void (*)(char, void*), void*, const char*, va_list)` | printf format engine |
| `0x10002fec` | `exit` | `void exit(int)` | C runtime exit |
| `0x10002ff4` | `runtime_init` | `void runtime_init(void)` | SDK runtime init |
| `0x10003020` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` | CRLF output driver |
| `0x10003130` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` | buffered string output |
| `0x1000321c` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` | enable the UART driver |
| `0x10003244` | `stdio_init_all` | `bool stdio_init_all(void)` | SDK serial init |
| `0x10003250` | `__wrap_getchar` | `int __wrap_getchar(void)` | the `getchar` wrapper (reads the UART) |
| `0x10003344` | `__wrap_puts` | `int __wrap_puts(const char*)` | the `puts` wrapper |
| `0x10003380` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` | printf core |
| `0x10003444` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` | the `printf` wrapper (`%s` calls) |
| `0x10003600` | `stdio_uart_init` | `void stdio_uart_init(void)` | SDK UART stdio init |
| `0x10003740` | `strlen` | `size_t strlen(const char*)` | C runtime string length |

> **`main` is `0x10000234` in both projects.** In Project 1 the three `static` helpers are inlined into `main`; in Project 2 `eval_if_else` and `process_servo_command` are inlined, and `sweep_servo` with them. That is why both projects put `main` at the same address. In a `Debug` build the helpers stay separate and `main` moves — another reason to build `Release`.

> **`printf` in our source is `__wrap_printf` in the binary.** The SDK links our `printf` calls to its `__wrap_printf` wrapper, which forwards to `__wrap_vprintf`. Project 1 has no `printf` at all: the compiler replaced every `printf("...")` with a `__wrap_puts` because the strings have no format specifiers.

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
2. Save it next to the image as `0x001d_static-conditionals.i64`.
3. From now on, save with `File -> Save` (`Cmd+S` on macOS, `Ctrl+S` on Windows/Linux) whenever you rename or patch.

| File | Role |
| ---- | ---- |
| `0x001d_static-conditionals.bin` | the raw firmware image; IDA never modifies it |
| `0x001d_static-conditionals.i64` | your analysis database: names, types, comments, and patches |

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

`Y` is the **Change Type** key, and it is what actually resolves the function — it turns `void sub_100017fc()` into `bool stdio_init_all(void)`. The Change Type dialog shows the full declaration (name and type), so typing the prototype sets both:

1. `G` to the function's address. The cursor lands on the function.
2. Press **`Y`**. In the Change Type dialog, type the prototype from the table exactly — for example `bool stdio_init_all(void)` — and press Enter.

If `Y` seems to do nothing, confirm the cursor is on the function, or right-click it and pick **Change Type...**. IDA parses what you type and silently keeps the old type if it does not parse, so glance at the header after each `Y`.

#### Worked example: `main`

1. Press `G`, type `0x10000234`, press Enter. The cursor lands on `sub_10000234`.
2. Press **`Y`** (Change Type), type `int main(void)`, press Enter.

> **IDA shows `int32_t` where Ghidra shows `int`.** After you set `int main(void)`, the decompiler header may read `int32_t main(void)`. That is the same type — on this platform `int` is 32 bits and IDA's parser normalises it to `int32_t`. Do not fight it; it is not an error.

#### Worked example: `servo_init`

1. `G` -> `0x1000027c`.
2. `Y` -> `void servo_init(uint8_t pin)`.

It takes a `uint8_t` pin number; `main` calls it with `6` (`movs r0, #6`).

#### Worked example: `servo_set_angle`

1. `G` -> `0x10000318`.
2. `Y` -> `void servo_set_angle(float degrees)`.

The float arrives in **`r0`** (soft-float ABI), not `s0`. The function opens with `vmov s14, r0` and then clamps the resulting pulse to `[1000, 2000]` — you can see `cmp.w r3, #2000` and `cmp.w r3, #1000` inside it.

#### Worked example: `__wrap_puts`

1. `G` -> `0x1000188c`.
2. `Y` -> `int __wrap_puts(const char *s)`.

Both prints in `main` land here. `printf("1\r\n")` has no format specifiers, so the compiler replaced it with `puts`; the `\n` was trimmed out of the string because `puts` adds one.

#### Worked example: `stdio_init_all`

1. `G` -> `0x100017fc`.
2. `Y` -> `bool stdio_init_all(void)`.

It returns **`bool`**, not `void` — the ELF says `_Bool stdio_init_all(void)`. Our `main` ignores the return value, so the decompiler still reads cleanly.

#### Worked example: `sleep_ms`

1. `G` -> `0x10000e28`.
2. `Y` -> `void sleep_ms(uint32_t ms)`.

Both delay instructions load `r0 = 0x1f4` (500) immediately before calling it.

The rest of the chain is the same two keystrokes per function (`G`, then `Y`). This is **our code plus the library functions it actually calls** — not the whole SDK.

The call chain for this project:

```
main
├── stdio_init_all ── stdio_uart_init ── gpio_set_function, uart_init, stdio_set_driver_enabled
│                                        └── uart_init ── clock_get_hz, busy_wait_us
├── servo_init ── gpio_set_function, clock_get_hz
├── __wrap_puts ── strlen, stdio_put_string ── time_us_64, strlen
└── servo_set_angle, sleep_ms
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
| `0x1000027c` | `servo_init` | `void servo_init(uint8_t)` |
| `0x10000318` | `servo_set_angle` | `void servo_set_angle(float)` |
| `0x100003d8` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` |
| `0x10000e28` | `sleep_ms` | `void sleep_ms(uint32_t)` |
| `0x1000100c` | `time_us_64` | `uint64_t time_us_64(void)` |
| `0x10001020` | `busy_wait_us` | `void busy_wait_us(uint64_t)` |
| `0x100010a0` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` |
| `0x10001274` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` |
| `0x10001604` | `exit` | `void exit(int)` |
| `0x1000160c` | `runtime_init` | `void runtime_init(void)` |
| `0x10001638` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` |
| `0x100016e8` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` |
| `0x100017d4` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` |
| `0x100017fc` | `stdio_init_all` | `bool stdio_init_all(void)` |
| `0x1000188c` | `__wrap_puts` | `int __wrap_puts(const char*)` |
| `0x10001a68` | `stdio_uart_init` | `void stdio_uart_init(void)` |
| `0x10001ba8` | `strlen` | `size_t strlen(const char*)` |

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
>     0x1000027c: ("servo_init",                "void servo_init(uint8_t)"),
>     0x10000318: ("servo_set_angle",           "void servo_set_angle(float)"),
>     0x100003d8: ("gpio_set_function",         "void gpio_set_function(uint, gpio_function_t)"),
>     0x10000e28: ("sleep_ms",                  "void sleep_ms(uint32_t)"),
>     0x1000100c: ("time_us_64",                "uint64_t time_us_64(void)"),
>     0x10001020: ("busy_wait_us",              "void busy_wait_us(uint64_t)"),
>     0x100010a0: ("uart_init",                 "uint uart_init(uart_inst_t*, uint)"),
>     0x10001274: ("clock_get_hz",              "unsigned long clock_get_hz(clock_handle_t)"),
>     0x10001604: ("exit",                      "void exit(int)"),
>     0x1000160c: ("runtime_init",              "void runtime_init(void)"),
>     0x10001638: ("stdio_out_chars_crlf",      "void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)"),
>     0x100016e8: ("stdio_put_string",          "int stdio_put_string(const char*, int, bool, bool)"),
>     0x100017d4: ("stdio_set_driver_enabled",  "void stdio_set_driver_enabled(stdio_driver_t*, bool)"),
>     0x100017fc: ("stdio_init_all",            "bool stdio_init_all(void)"),
>     0x1000188c: ("__wrap_puts",               "int __wrap_puts(const char*)"),
>     0x10001a68: ("stdio_uart_init",           "void stdio_uart_init(void)"),
>     0x10001ba8: ("strlen",                    "size_t strlen(const char*)"),
> }
> for addr, (name, sig) in funcs.items():
>     bv.define_user_symbol(Symbol(SymbolType.FunctionSymbol, addr, name))
>     f = bv.get_function_at(addr)
>     if f is not None:
>         f.set_user_type(sig)
> ```
>
> SDK type names (`stdio_driver_t`, `uart_inst_t`, `gpio_function_t`, plus `uint`, `va_list`, `clock_handle_t`) are **not** in the raw `.bin`. `set_user_type` re-parses each signature as C, so an undefined name raises `SyntaxError: unknown type name '...'` and stops the loop — it is not harmless. The `sdk` block above defines them first. Standard names (`uint8_t`, `uint32_t`, `uint64_t`, `bool`, `size_t`) are built in.

### Step 17: Read `main` in the decompiler

Open the **Decompiler** view on `main`:

```asm
10000234 <main>:
10000234: push  {r3, r4, r5, lr}
10000236: bl    100017fc <stdio_init_all>
1000023a: movs  r0, #6
1000023c: bl    1000027c <servo_init>
10000240: movs  r5, #0
10000242: ldr   r4, [pc, #44]
10000244: ldr   r0, [pc, #44]
10000246: bl    1000188c <__wrap_puts>
1000024a: ldr   r0, [pc, #44]
1000024c: bl    1000188c <__wrap_puts>
10000250: mov   r0, r5
10000252: bl    10000318 <servo_set_angle>
10000256: mov.w r0, #500
1000025a: bl    10000e28 <sleep_ms>
1000025e: mov   r0, r4
10000260: bl    10000318 <servo_set_angle>
10000264: mov.w r0, #500
10000268: bl    10000e28 <sleep_ms>
1000026c: b.n   10000244 <main+0x10>
1000026e: nop
10000270: .word 0x43340000
10000274: .word 0x10001c5c
10000278: .word 0x10001c64
```

The whole program is one loop because the three `static` helpers were inlined:

- **No `cmp` anywhere.** `choice` is the constant `1`, so the compiler folded `if (choice == 1)` to always-true, deleted `else if (choice == 2)` and `else`, and left only the `1` and `one` prints. That is the static conditional.
- `r5 = 0` (`movs r5, #0` at `0x10000240`) is the `0.0f` angle; `r4` holds `0x43340000` (180.0f) from the literal pool at `0x10000270`.
- `0x10000274` and `0x10000278` point at `"1\r\n"` and `"one\r"` in `.rodata`.

The decompiler reads roughly:

```c
int32_t main(void)
{
    stdio_init_all();
    servo_init(6);
    do
    {
        __wrap_puts("1\r\n");
        __wrap_puts("one\r");
        servo_set_angle(0.0f);
        sleep_ms(0x1f4);
        servo_set_angle(180.0f);
        sleep_ms(0x1f4);
    } while (true);
}
```

### Step 18: Patch 1 — change the strings `1` to `2` and `one` to `fun`

The strings live in `.rodata`:

```asm
10000270: .word 0x43340000
10000274: .word 0x10001c5c
10000278: .word 0x10001c64
```

`0x10001c5c` holds `31 0d 00` = `"1\r"`, and `0x10001c64` holds `6f 6e 65 0d 00` = `"one\r"`. Change the first byte of each string in the **Hex** view (`View -> Hex`, lock off) or the Python console:

```python
bv.write(0x10001c5c, b"\x32")  # "1" -> "2"
bv.write(0x10001c64, b"fun")  # "one" -> "fun"
print(bv.read(0x10001c5c, 4))  # -> b'2\r\x00\x00'
print(bv.read(0x10001c64, 6))  # -> b'fun\r\x00\x00'
```

Keep the replacement lengths identical: `"1"` is one byte, `"one"` is three. A shorter string must be padded and a longer one would run into the next string.

### Step 18b: Patch 2 — change the angle from 180.0f to 30.0f

The 180.0f literal sits at `0x10000270`:

```asm
10000270: .word 0x43340000
```

It is loaded into `r4` at `0x10000242` and copied into `r0` before the second `servo_set_angle`. IEEE-754:

- `0x43340000` = 180.0f → little-endian bytes `00 00 34 43`
- `0x41f00000` = 30.0f  → little-endian bytes `00 00 f0 41`

**Option A — Hex view:** go to `0x10000270` and change `00 00 34 43` to `00 00 f0 41`, then reanalyze.

**Option B — Python console:**

```python
bv.write(0x10000270, bytes.fromhex("0000f041"))
print(bv.read(0x10000270, 4).hex(" "))  # -> 00 00 f0 41
```

`30.0 = 1.875 × 2^4`; sign `0`, exponent `127 + 4 = 131 = 0x83`, mantissa `0.875 = 0x700000` → `0x41f00000`.

### Step 18c: Patch 3 — speed up the sweep from 500 ms to 100 ms

The compiler packed `500` directly into two 32-bit Thumb-2 `mov.w` instructions:

```asm
10000256: mov.w r0, #500
1000025a: bl    10000e28 <sleep_ms>
10000264: mov.w r0, #500
10000268: bl    10000e28 <sleep_ms>
```

Each `mov.w r0, #500` is the four bytes `4f f4 fa 70`. The four bytes for `mov.w r0, #100` are `4f f0 64 00` (verified by assembling `mov.w r0, #100` with `arm-none-eabi-as`). Change both:

```python
for addr in (0x10000256, 0x10000264):
    bv.write(addr, bytes.fromhex("4ff06400"))  # mov.w r0, #100
```

> **Why the bytes change shape.** `500` does not fit in an 8-bit rotated immediate, so the encoder uses the `f4 4f`-family form `4f f4 fa 70`. `100` (`0x64`) does fit, so the encoder uses the `f04f`/`f0 4f` form `4f f0 64 00`. Same instruction, different immediate encoding. Both are exactly 4 bytes.

Verify all five patches:

```python
for addr in (0x10001c5c, 0x10001c64, 0x10000270, 0x10000256, 0x10000264):
    print(hex(addr), bv.read(addr, 4).hex(" "))
# -> 0x10001c5c 32 0d 00 00
# -> 0x10001c64 66 75 6e 0d
# -> 0x10000270 00 00 f0 41
# -> 0x10000256 4f f0 64 00
# -> 0x10000264 4f f0 64 00
```

### Step 19: Export the patched `.bin`

```python
import os
seg = next(s for s in bv.segments if s.data_length)  # the loadable image segment
data = bv.read(seg.start, seg.data_length)  # base + size come from the view itself
out = os.path.join(os.path.join(root, "0x001d_static-conditionals", "build"), "0x001d_static-conditionals-h.bin")
open(out, "wb").write(data)
print(len(data), out)  # -> 8084 /.../build/0x001d_static-conditionals-h.bin
```

Where the two numbers come from — nothing is hardcoded:

- **`seg.start`** is the image base IDA loaded the `.bin` at (`0x10000000`), the same value you pass to `uf2conv --base`.
- **`seg.data_length`** is the segment's size in the file (`0x1f94` = 8084). Exactly one segment carries data (the image); every peripheral and synthetic segment has `data_length == 0`, so `next(...)` picks the image.

> **No relative path.** IDA's Python console runs with a read-only working directory (inside the app bundle), so a relative `open(...)` fails with `OSError: [Errno 30] Read-only file system`. `root` (from `~/.embedded-hacking-repo`, Step 3) is the repo, so the file is written into the project's `build/`.

### Step 20: Convert to UF2

Run from the project directory:

**macOS Apple Silicon / Linux x64:**

```bash
python3 ../uf2conv.py 0x001d_static-conditionals-h.bin \
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

**Windows x64:**

```cmd
python ..\uf2conv.py 0x001d_static-conditionals-h.bin ^
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

> **Or convert from the IDA console** — `chdir` to a writable directory first (the default one is read-only), then run the script:
>
> ```python
> import os, sys, runpy
> os.chdir(os.path.join(root, "0x001d_static-conditionals", "build"))  # the project build dir (writable)
> sys.argv = ["uf2conv.py", "0x001d_static-conditionals-h.bin",
>             "--base", "0x10000000", "--family", "0xe48bff59", "--output", "hacked.uf2"]
> runpy.run_path("../../uf2conv.py", run_name="__main__")
> ```

## Part 5: Static — Resolve the Functions and Patch (Project 2)

### Step 26: Resolve the functions in the IDA GUI

Same two keys as Step 16 — `G` to the address, then `Y` (Change Type) to set the prototype — using the Project 2 ELF symbol map from Step 4.

#### Worked example: `main`

1. `G` -> `0x10000234`.
2. `Y` -> `int main(void)` (IDA shows `int32_t main(void)` — the same 32-bit `int`).

#### Worked example: `__wrap_getchar`

1. `G` -> `0x10003250`.
2. `Y` -> `int __wrap_getchar(void)`.

`getchar` returns an `int` in `r0`; `main` immediately narrows it with `uxtb r0, r0` before comparing.

#### Worked example: `__wrap_printf`

1. `G` -> `0x10003444`.
2. `Y` -> `int __wrap_printf(const char *fmt, ...)`. Keep the `...` — `printf` is variadic. It forwards to `__wrap_vprintf`.

#### Worked example: `__wrap_puts`

1. `G` -> `0x10003344`.
2. `Y` -> `int __wrap_puts(const char *s)`.

#### Worked example: `servo_set_angle`

1. `G` -> `0x10000378`.
2. `Y` -> `void servo_set_angle(float degrees)`.

The clamp constants are inside it:

```asm
100003ba: cmp.w   r3, #2000
100003be: it      cs
100003c0: movcs.w r3, #2000
100003c4: cmp.w   r3, #1000
100003c8: it      cc
100003ca: movcc.w r3, #1000
```

`0x7d0` is the 2000 µs maximum pulse and `0x3e8` is the 1000 µs minimum.

The call chain for this project:

```
main
├── stdio_init_all ── stdio_uart_init ── gpio_set_function, uart_init, stdio_set_driver_enabled
│                                        └── uart_init ── clock_get_hz, busy_wait_us
├── servo_init ── gpio_set_function, clock_get_hz
├── __wrap_getchar ── busy_wait_us
├── __wrap_puts ── strlen, stdio_put_string ── time_us_64, strlen
├── __wrap_printf ── __wrap_vprintf ── vfctprintf, stdio_out_chars_crlf, time_us_64
└── servo_set_angle, sleep_ms
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
| `0x100002dc` | `servo_init` | `void servo_init(uint8_t)` |
| `0x10000378` | `servo_set_angle` | `void servo_set_angle(float)` |
| `0x10000438` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` |
| `0x10000e88` | `sleep_ms` | `void sleep_ms(uint32_t)` |
| `0x1000106c` | `time_us_64` | `uint64_t time_us_64(void)` |
| `0x10001080` | `busy_wait_us` | `void busy_wait_us(uint64_t)` |
| `0x10001100` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` |
| `0x100012d4` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` |
| `0x10002f90` | `vfctprintf` | `int vfctprintf(void (*)(char, void*), void*, const char*, va_list)` |
| `0x10002fec` | `exit` | `void exit(int)` |
| `0x10002ff4` | `runtime_init` | `void runtime_init(void)` |
| `0x10003020` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` |
| `0x10003130` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` |
| `0x1000321c` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` |
| `0x10003244` | `stdio_init_all` | `bool stdio_init_all(void)` |
| `0x10003250` | `__wrap_getchar` | `int __wrap_getchar(void)` |
| `0x10003344` | `__wrap_puts` | `int __wrap_puts(const char*)` |
| `0x10003380` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` |
| `0x10003444` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` |
| `0x10003600` | `stdio_uart_init` | `void stdio_uart_init(void)` |
| `0x10003740` | `strlen` | `size_t strlen(const char*)` |

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
>     0x100002dc: ("servo_init",                "void servo_init(uint8_t)"),
>     0x10000378: ("servo_set_angle",           "void servo_set_angle(float)"),
>     0x10000438: ("gpio_set_function",         "void gpio_set_function(uint, gpio_function_t)"),
>     0x10000e88: ("sleep_ms",                  "void sleep_ms(uint32_t)"),
>     0x1000106c: ("time_us_64",                "uint64_t time_us_64(void)"),
>     0x10001080: ("busy_wait_us",              "void busy_wait_us(uint64_t)"),
>     0x10001100: ("uart_init",                 "uint uart_init(uart_inst_t*, uint)"),
>     0x100012d4: ("clock_get_hz",              "unsigned long clock_get_hz(clock_handle_t)"),
>     0x10002f90: ("vfctprintf",                "int vfctprintf(void (*)(char, void*), void*, const char*, va_list)"),
>     0x10002fec: ("exit",                      "void exit(int)"),
>     0x10002ff4: ("runtime_init",              "void runtime_init(void)"),
>     0x10003020: ("stdio_out_chars_crlf",      "void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)"),
>     0x10003130: ("stdio_put_string",          "int stdio_put_string(const char*, int, bool, bool)"),
>     0x1000321c: ("stdio_set_driver_enabled",  "void stdio_set_driver_enabled(stdio_driver_t*, bool)"),
>     0x10003244: ("stdio_init_all",            "bool stdio_init_all(void)"),
>     0x10003250: ("__wrap_getchar",            "int __wrap_getchar(void)"),
>     0x10003344: ("__wrap_puts",               "int __wrap_puts(const char*)"),
>     0x10003380: ("__wrap_vprintf",            "int __wrap_vprintf(const char*, va_list)"),
>     0x10003444: ("__wrap_printf",             "int __wrap_printf(const char*, ...)"),
>     0x10003600: ("stdio_uart_init",           "void stdio_uart_init(void)"),
>     0x10003740: ("strlen",                    "size_t strlen(const char*)"),
> }
> for addr, (name, sig) in funcs.items():
>     bv.define_user_symbol(Symbol(SymbolType.FunctionSymbol, addr, name))
>     f = bv.get_function_at(addr)
>     if f is not None:
>         f.set_user_type(sig)
> ```

### Step 27: Patch 1 — change the servo angle from 180.0f to 30.0f

The 180.0f literal sits at `0x100002c0`, loaded into `r5` at `0x10000242`:

```asm
100002c0: .word 0x43340000
```

Change the four bytes exactly as in Project 1:

```python
bv.write(0x100002c0, bytes.fromhex("0000f041"))  # 180.0f -> 30.0f
print(bv.read(0x100002c0, 4).hex(" "))  # -> 00 00 f0 41
```

Both servo paths (`0x10000284`/`0x100002a2` use `r5`, `0x10000276`/`0x100002b0` use `r4` = 0.0f) now cap at 30°.

### Step 27b: Patch 2 — rewrite the secret keys `1` → `x` and `2` → `y`

The two key comparisons are at `0x1000024a` and `0x1000024e`:

```asm
1000024a: cmp r0, #49
1000024e: cmp r0, #50
```

The immediate is the low byte of the 16-bit `cmp` encoding: `0x31` at `0x1000024a` and `0x32` at `0x1000024e`. Change them to `x` (`0x78`) and `y` (`0x79`):

```python
bv.write(0x1000024a, b"\x78")  # cmp r0, #0x78  ('x')
bv.write(0x1000024e, b"\x79")  # cmp r0, #0x79  ('y')
print(bv.read(0x1000024a, 2).hex(" "))  # -> 78 28
print(bv.read(0x1000024e, 2).hex(" "))  # -> 79 28
```

Now `x` takes the old `1` path and `y` takes the old `2` path.

### Step 27c (optional): Patch 3 — make `x` and `y` stealth (skip the prints)

The original `1`/`2` paths print before moving the servo. To make the new `x`/`y` keys silent, redirect the two `beq.n` targets straight to the servo code, skipping both prints. The current targets are `0x10000268` (the `1` print block) and `0x10000294` (the `2` print block); the servo code starts at `0x10000276` (`mov r0, r4`) and `0x100002a2` (`mov r0, r5`).

| Address | Instruction | Before | After | New target |
| ------- | ----------- | ------ | ----- | ---------- |
| `0x1000024c` | `beq.n` | `0c d0` | `13 d0` | `0x10000276` (skip `1`/`one` prints) |
| `0x10000250` | `beq.n` | `20 d0` | `27 d0` | `0x100002a2` (skip `2`/`two` prints) |

```python
bv.write(0x1000024c, bytes.fromhex("13d0"))  # beq.n -> 0x10000276
bv.write(0x10000250, bytes.fromhex("27d0"))  # beq.n -> 0x100002a2
```

> **How the encoding was chosen.** A 16-bit conditional branch is `1101 cond imm8`; the target is `PC + 4 + (imm8 << 1)`. For `0x1000024c` → `0x10000276`: `(0x276 - 0x250) / 2 = 0x13`. For `0x10000250` → `0x100002a2`: `(0x2a2 - 0x254) / 2 = 0x27`. Both were verified by patching a copy of the raw `.bin` and disassembling it with `arm-none-eabi-objdump`. This patch is optional; the key rewrite in Step 27b works without it (it just still prints).

### Step 28: Export, convert and export

```python
import os
seg = next(s for s in bv.segments if s.data_length)  # the loadable image segment
data = bv.read(seg.start, seg.data_length)  # base + size from the view itself
out = os.path.join(os.path.join(root, "0x0020_dynamic-conditionals", "build"), "0x0020_dynamic-conditionals-h.bin")
open(out, "wb").write(data)
print(len(data), out)  # -> 16188 /.../build/0x0020_dynamic-conditionals-h.bin
```

`seg.data_length` is the image size (`0x3f3c` = 16188) read from the view — nothing hardcoded.

**macOS Apple Silicon / Linux x64:**

```bash
python3 ../uf2conv.py 0x0020_dynamic-conditionals-h.bin \
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

**Windows x64:**

```cmd
python ..\uf2conv.py 0x0020_dynamic-conditionals-h.bin ^
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

Or run the conversion from the IDA console, exactly as in Step 20 (`os.chdir` to the build dir, then `runpy.run_path("../../uf2conv.py", run_name="__main__")` with `sys.argv` set to the arguments above).

Hold **BOOTSEL**, plug in the Pico 2, drag `hacked.uf2` onto the **`RP2350`** drive. Or flash the `.bin` over the Debug Probe with SWD — no BOOTSEL — from the console, exactly as in Step 21.

## Glossary

| Term | Definition |
| ---- | ---------- |
| **`beq`** | Branch if Equal — ARM conditional jump, taken when the Z flag is set |
| **`bne`** | Branch if Not Equal — ARM conditional jump, taken when the Z flag is clear |
| **`.bss`** | Section for uninitialized global variables; zeroed by startup code |
| **`.data`** | Section for initialized global variables; copied from flash to SRAM at boot |
| **Dynamic conditional** | A condition whose value is only known at run time (e.g. `getchar()`), so the compiler must emit the comparisons and branches |
| **`.elf`** | Linked image with the symbol table; the ground truth for addresses and names |
| **GPIO** | General Purpose Input/Output — controllable pins on the microcontroller |
ly flash |
| **Inlining** | The optimizer replacing a function call with the function body; why the `static` helpers disappear from `main` in `Release` |
| **Literal pool** | A block of 32-bit constants that Thumb-2 code reaches with PC-relative `ldr` |
| **PWM** | Pulse Width Modulation — a variable pulse-width signal; 50 Hz for the SG90 |
| **`.rodata`** | Read-only section for constants and string literals; stays in flash |
| **SG90** | A common 0°–180° hobby servo driven by a 1–2 ms pulse every 20 ms |
| **SIO** | Single-cycle I/O — the fast GPIO block in the RP2350, at `0xd0000000` |
| **Static conditional** | A condition whose value is known at compile time, so the compiler folds it and deletes dead branches |
| **Thumb bit** | Bit 0 of a Cortex-M function pointer; selects Thumb instruction mode |
| **UF2** | USB Flashing Format — the file format the Pico 2 bootloader accepts |
| **Vector table** | The first words of flash: initial stack pointer and exception vectors |

---

**Remember:** the ELF tells you what every address is, and the `.bin` is what you actually patch. Verify the static behavior from the recovered instructions.

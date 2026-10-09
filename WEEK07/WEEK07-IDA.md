# Week 7-IDA: IDA Pro — Read, Hack, and Patch Constants and a 1602 LCD String (Raw `.bin`)

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

- Build the lesson project with `Release` and get both an `.elf` and a raw `.bin`
- Dump the **ELF symbol map** with `arm-none-eabi-nm` and use it as ground truth
- Load the raw `.bin` into IDA at `0x10000000`
- Read how a `#define` macro **and** a `const` variable both fold to bare instruction immediates
- Tell a 16-bit `movs r1, #42` from a 32-bit `movw r1, #1337` in the disassembly
- Follow the I2C path through the `i2c_inst_t` struct: `I2C_PORT` -> `i2c1` -> `&i2c1_inst` -> `hw` -> `0x40098000`
- **Break at the `printf` call** on live silicon and hack the printed constant live
- Optionally **hack the LCD string live** by pointing `r0` at a RAM replacement
- **Resolve the functions in the IDA GUI** using the ELF symbol map, including the `lcd_1602.c` driver symbols
- **Patch** both constants (`42 -> 43`, `1337 -> 1344`) and the LCD string (`"Reverse" -> "Exploit"`)
- **Export** the patched image, convert it to UF2, and flash it

---

## How This Guide Works

The build produces two files for the project:

| File | What it is | How we use it |
| ---- | ---------- | ------------- |
| `.elf` | The linked image with a full symbol table | Ground truth for every function address and name |
| `.bin` | The raw flash image, no headers, no symbols | The image we load into IDA and reverse |

The `.bin` is built **from** the `.elf`, so the ELF tells you exactly what is at every address. We use the ELF symbol map to resolve functions in IDA, and we reverse-engineer the raw `.bin` the way a real extracted firmware image is reversed.

> **Build `Release`, not `Debug`.** Every address in this guide matches the Week 7 lesson, and the Week 7 lesson is a `Release` build. `Release` optimizes the code the same way the original lesson was built: it **inlines** the `static` helpers `init_i2c_and_lcd` and `write_lcd_greeting` (and the whole `lcd_1602.c` static helper chain) straight into `main` or into the public `lcd_*` functions, and it folds both `FAV_NUM` and `OTHER_FAV_NUM` down to immediate values. If you build `Debug`, the SDK function addresses move and the helpers stay separate calls, so nothing lines up. Always build `Release` for this lesson.

The order is **dynamic first, static second**:

1. Break on the live target and prove what the code does.
2. Hack it live in the debugger and watch the output change.
3. Resolve the functions in IDA using the ELF symbol map.
4. Patch the bytes, export, convert, and flash.

| Project | Serial output | Also does | The hacks |
| ------- | ------------- | --------- | --------- |
| `0x0017_constants` | `FAV_NUM: 42`, `OTHER_FAV_NUM: 1337` | I2C1 (SDA GP2, SCL GP3) drives a 1602 LCD, writing `Reverse` / `Engineering` | `42 -> 43`, `1337 -> 1344`, and `"Reverse" -> "Exploit"` |

> **Addresses come from your build.** Every address here is from the `Release` build produced in Step 3. Confirm against your own `.elf` with the command in Step 4.

> **The surprise of this week is that `const` is not in memory.** `#define FAV_NUM 42` becomes the 16-bit `movs r1, #42`; `const int OTHER_FAV_NUM = 1337` *also* becomes an immediate, the 32-bit `movw r1, #1337`. The compiler only keeps a `const` in `.rodata` if the program **takes its address** (`&OTHER_FAV_NUM`); this program never does, so the `const` is inlined exactly like the macro. You patch an instruction operand, not a data word.

---

## Part 1: Build, Flash, and Get the Symbol Map

### Step 1: Install the toolchain

**Windows x64**

- Install the **Raspberry Pi Pico** extension in VS Code. It installs the ARM GNU toolchain, CMake, Ninja, and the Pico SDK.
- Install **IDA Pro** and complete its license activation.
- Install **PuTTY** for the serial monitor.

**macOS Apple Silicon**

```bash
brew install cmake ninja
```

- Install **IDA Pro** and complete its license activation.
- Install the **Arm GNU Toolchain**, or let the VS Code Pico extension manage it.

**Linux x64**

```bash
sudo apt install cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi git python3 openocd minicom
```

- Install **IDA Pro** and complete its license activation.

### Step 2: Verify your tools are the right architecture (do not skip this)

On **macOS Apple Silicon**, the most common failure is an Intel `x86_64` tool on your `PATH`:

```
zsh: bad CPU type in executable: cmake
```

You may have **two Homebrews**: the arm64 one at `/opt/homebrew` and the Intel one at `/usr/local`. If `/usr/local/bin` wins, every `brew` tool is x86_64. Check:

```bash
file "$(which cmake)"
file "$(which ninja)"
file "$(which arm-none-eabi-gdb)"
file "$(which arm-none-eabi-nm)"
file "$(which openocd)"
file "$(which telnet)"
```

All must report `arm64`. If any is `x86_64`, put the Apple Silicon prefix first for the session and check again:

```bash
export PATH="/opt/homebrew/bin:$PATH"
hash -r
file "$(which cmake)"
```

To make it permanent, add that `export` to `~/.zshrc`. Do not use Rosetta as a fix; OpenOCD and GDB are exactly the kind of programs where a translation layer produces failures that look like debugger bugs.

**`telnet` is special — and optional.** The GDB MI workflow does not need it; it is only used by the command-port fallback. macOS no longer ships `telnet`, and the Homebrew build is often the Intel one, so `telnet 127.0.0.1 4444` fails with `bad CPU type in executable`. Call the Apple Silicon Homebrew explicitly:

```bash
/opt/homebrew/bin/brew install telnet
```

If you would rather not install anything, macOS ships an arm64 `nc`, which can connect to the same OpenOCD port:

```bash
nc 127.0.0.1 4444
```

**Windows x64** and **Linux x64** do not have this problem. Skip to Step 3.

### Step 3: Build the project with `Release`

Run this inside `0x0017_constants/`:

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
proj = os.path.join(root, "0x0017_constants")
subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

**Linux x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
proj = os.path.join(root, "0x0017_constants")
subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

**Windows x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
proj = os.path.join(root, "0x0017_constants")
subprocess.run(["cmake", "-B", "build", "-G", "Ninja", "-DPICO_BOARD=pico2",
                "-DPICO_PLATFORM=rp2350", "-DCMAKE_BUILD_TYPE=Release"], cwd=proj)
subprocess.run(["cmake", "--build", "build"], cwd=proj)
```

The build directory now contains the pair we need:

- `0x0017_constants/build/0x0017_constants.elf` and `.bin` — the `.bin` is **17980** bytes (`0x463c`)

If the ARM toolchain is not on your `PATH`, add `-DPICO_TOOLCHAIN_PATH=...`:

| OS | Typical toolchain path |
| -- | ---------------------- |
| Windows x64 | `C:/Program Files/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin` |
| macOS Apple Silicon | `~/.pico-sdk/toolchain/14_2_Rel1/bin` |
| Linux x64 | `/usr` |

> **This guide's toolchain lives at `~/.pico-sdk/toolchain/14_2_Rel1/bin`.** All of `arm-none-eabi-nm`, `arm-none-eabi-objdump`, and `arm-none-eabi-gdb` resolved in this document come from there. If your install is elsewhere, `which arm-none-eabi-nm` tells you where to point.

### Step 4: Dump the ELF symbol map

This is the ground truth for the whole lesson. Run `arm-none-eabi-nm` on the ELF and keep the output in a terminal or a text file:

**macOS Apple Silicon / Linux x64:**

```bash
arm-none-eabi-nm -n --defined-only build/0x0017_constants.elf | grep -E ' [Tt] '
```

**Windows x64:**

```powershell
arm-none-eabi-nm -n --defined-only build\0x0017_constants.elf | Select-String ' [Tt] '
```

Each line is `address type name`. The `T`/`t` type is a function. Here are the functions this lesson uses. The signatures come from the ELF's DWARF debug info queried with `arm-none-eabi-gdb -batch -ex "ptype <name>"`, so they are exact.

**Our code and the startup chain:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` | reset entry |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` | calls `runtime_init`, `main`, `exit` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` | copies `.data` from flash to SRAM |
| `0x100001e4` | `_init` | `void _init(void)` | runs `.init_array` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` | C runtime boilerplate |
| `0x10000234` | `main` | `int main(void)` | the lesson function (`init_i2c_and_lcd` and `write_lcd_greeting` inlined) |

**Our I2C/LCD driver (`lcd_1602.c`) and the SDK I2C functions `main` reaches:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x100002bc` | `lcd_i2c_init` | `void lcd_i2c_init(i2c_inst_t*, uint8_t, int, uint8_t)` | store config + HD44780 reset/configure |
| `0x100006f4` | `lcd_set_cursor` | `void lcd_set_cursor(int, int)` | move the HD44780 cursor |
| `0x100007f0` | `lcd_puts` | `void lcd_puts(const char*)` | write a string to the LCD |
| `0x10003cdc` | `i2c_init` | `uint i2c_init(i2c_inst_t*, uint)` | SDK I2C init (100 kHz) |
| `0x10003d28` | `i2c_write_blocking` | `int i2c_write_blocking(i2c_inst_t*, uint8_t, const uint8_t*, size_t, bool)` | one blocking I2C transfer |
| `0x100008f0` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` | SDK GPIO function select (I2C pins) |
| `0x1000092c` | `gpio_set_pulls` | `void gpio_set_pulls(uint, bool, bool)` | SDK pull config (`gpio_pull_up` inlined) |

> The `lcd_1602.c` static helpers — `pcf_write_byte`, `pcf_pulse_enable`, `lcd_write4`, `lcd_send`, `lcd_store_config`, `lcd_hd44780_reset`, `lcd_hd44780_configure` — have **no symbol of their own** in the `Release` build. They are inlined into `lcd_i2c_init`, `lcd_set_cursor`, and `lcd_puts`, which is why those three functions are large and call `i2c_write_blocking` and the `sleep_*` helpers directly.

**The stdio/UART and printf chain `main` reaches:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x10001368` | `sleep_us` | `void sleep_us(uint64_t)` | SDK microsecond delay |
| `0x10001440` | `sleep_ms` | `void sleep_ms(uint32_t)` | SDK millisecond delay |
| `0x10001624` | `time_us_64` | `uint64_t time_us_64(void)` | SDK microsecond clock |
| `0x10001638` | `busy_wait_us` | `void busy_wait_us(uint64_t)` | UART timing loop |
| `0x100016b8` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` | SDK UART init |
| `0x1000188c` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` | UART clock lookup |
| `0x100035a4` | `exit` | `void exit(int)` | C runtime exit |
| `0x100035ac` | `runtime_init` | `void runtime_init(void)` | SDK runtime init |
| `0x100035d8` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` | CRLF output driver |
| `0x100036e8` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` | buffered string output |
| `0x100037d4` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` | enable the UART driver |
| `0x100037fc` | `stdio_init_all` | `bool stdio_init_all(void)` | SDK serial init |
| `0x100038c8` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` | printf core |
| `0x1000398c` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` | the `printf` wrapper |
| `0x10003b48` | `stdio_uart_init` | `void stdio_uart_init(void)` | SDK UART stdio init |
| `0x10003e24` | `strlen` | `size_t strlen(const char*)` | C runtime string length |

**SDK helpers the `printf` path reaches:**

| Address | ELF symbol | Signature | Role |
| ------- | ---------- | --------- | ---- |
| `0x10003548` | `vfctprintf` | `int vfctprintf(void (*)(char, void*), void*, const char*, va_list)` | printf format engine |
| `0x10002b64` | `_vsnprintf` | `int _vsnprintf(out_fct_type, char*, size_t, const char*, va_list)` | the format dispatcher |
| `0x10001f48` | `_ntoa_format` | `unsigned _ntoa_format(out_fct_type, char*, size_t, size_t, char*, size_t, bool, unsigned, unsigned, unsigned, unsigned)` | number formatter |
| `0x10001eac` | `_out_rev` | `unsigned _out_rev(out_fct_type, char*, size_t, size_t, const char*, size_t, unsigned, unsigned)` | reversed-digit output |
| `0x1000211c` | `_out_char` | `void _out_char(char, void*, size_t, size_t)` | single-char sink |

Two things in this project have **no symbol of their own**, because the compiler inlined them into `main`:

- `init_i2c_and_lcd` and `write_lcd_greeting` — the two `static` helpers in our own source are inlined, so there is no address to rename. You see their bodies directly inside `main`.
- `gpio_pull_up` — `static inline` in the SDK, so `gpio_pull_up(2)` and `gpio_pull_up(3)` compile to direct calls to `gpio_set_pulls` at `0x10000258` and `0x10000262`.

### Step 5: Flash and confirm the output

A `.bin` has no headers, so OpenOCD must be told the base address `0x10000000`. From the repository root:

**macOS Apple Silicon / Linux x64:**

```bash
./flash.sh 0x0017_constants/build/0x0017_constants.bin
```

**Windows x64 (PowerShell):**

```powershell
.\flash.ps1 -Bin 0x0017_constants\build\0x0017_constants.bin
```

**Or flash from the IDA console** (the console reads the repo root from the marker file, so it works with no database open):

**macOS Apple Silicon / Linux x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()  # set once (Step 3)
bin_path = os.path.join(root, "0x0017_constants", "build", "0x0017_constants.bin")
log = os.path.join(os.path.dirname(bin_path), "flash.log")
subprocess.run(["pkill", "-TERM", "-f", "openocd"])  # free the probe first
subprocess.Popen([os.path.join(root, "flash.sh"), bin_path],
                 stdout=open(log, "w"), stderr=subprocess.STDOUT, start_new_session=True)
print("flashing in the background; log:", log)
```

**Windows x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()  # set once (Step 3)
bin_path = os.path.join(root, "0x0017_constants", "build", "0x0017_constants.bin")
log = os.path.join(os.path.dirname(bin_path), "flash.log")
subprocess.run(["taskkill", "/F", "/IM", "openocd.exe"])  # free the probe first
subprocess.Popen(["powershell", "-ExecutionPolicy", "Bypass", "-File",
                  os.path.join(root, "flash.ps1"), "-Bin", bin_path],
                 stdout=open(log, "w"), stderr=subprocess.STDOUT)
print("flashing in the background; log:", log)
```

Wait for `wrote 17980 bytes ...` and `** Verified OK **`. Open a serial monitor at **115200** baud:

- **Windows x64:** PuTTY -> Connection type **Serial**, the Pico's COM port, speed `115200`.
- **macOS Apple Silicon:** `screen /dev/tty.usbmodem* 115200` (quit with `Ctrl-A` then `K`).
- **Linux x64:** `minicom -D /dev/ttyACM0 -b 115200`.

```
FAV_NUM: 42
OTHER_FAV_NUM: 1337
FAV_NUM: 42
OTHER_FAV_NUM: 1337
...
```

The 1602 LCD shows `Reverse` on line 1 and `Engineering` on line 2. Both numbers print forever, because both constants are baked into the loop as immediates. That is the behavior we will change.

---

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

### Step 8: The views you will use

- **Linear view:** the disassembly listing. You navigate, read, and patch here.
- **Graph view:** the control-flow graph of the current function.
- **Decompiler (HLIL):** the pseudo-C decompilation.
- **Hex view:** raw bytes, used for patching.
- **Function list:** the sidebar list of every detected function.

Navigation: `G` go to address, `N` rename, `Y` set type or signature, `;` add a comment. Breakpoints are set from the GUI through the GDB MI adapter — see Step 12.

> **macOS function keys:** the top-row `F` keys are usually mapped to system functions. Every step here uses menu paths that work without them.

---

## Part 3: Dynamic — Break at the `printf` Call and Hack Live

### Step 9: Start OpenOCD as a live debug server

Make sure no other OpenOCD is running; a forgotten server holds port `3333`.

**macOS / Linux:**

```bash
ps aux | grep -i openocd
```

**Windows (PowerShell):**

```powershell
Get-Process | Where-Object { $_.ProcessName -like '*openocd*' }
```

Stop any leftover server gracefully:

**macOS / Linux:**

```bash
pkill -TERM -f openocd
```

**Windows (PowerShell):**

```powershell
Get-Process openocd -ErrorAction SilentlyContinue | Stop-Process
```

Start the server **parked at `main`**:

**macOS Apple Silicon / Linux x64:**

```bash
BP_ADDR=0x10000234 ./debug-server.sh
```

**Windows x64 (PowerShell):**

```powershell
$env:BP_ADDR="0x10000234"; .\debug-server.ps1
```

**Or start it from the IDA console**, freeing the probe first and launching the server in the background so the console returns immediately:

**macOS Apple Silicon / Linux x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()  # set once (Step 3)
subprocess.run(["pkill", "-TERM", "-f", "openocd"])  # stop any running server first
log = os.path.join(root, "openocd.log")
p = subprocess.Popen([os.path.join(root, "debug-server.sh")], cwd=root,
                     env=dict(os.environ, BP_ADDR="0x10000234"),
                     stdout=open(log, "w"), stderr=subprocess.STDOUT, start_new_session=True)
print("OpenOCD started (pid", p.pid, "); log:", log)
```

**Windows x64:**

```python
import os, subprocess
root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()  # set once (Step 3)
subprocess.run(["taskkill", "/F", "/IM", "openocd.exe"])  # stop any running server first
log = os.path.join(root, "openocd.log")
p = subprocess.Popen(["powershell", "-ExecutionPolicy", "Bypass", "-File",
                      os.path.join(root, "debug-server.ps1")], cwd=root,
                     env=dict(os.environ, BP_ADDR="0x10000234"),
                     stdout=open(log, "w"), stderr=subprocess.STDOUT)
print("OpenOCD started (pid", p.pid, "); log:", log)
```

`Popen` returns in a few milliseconds; the server keeps running in the background. Check `openocd.log` for `Listening on port 3333`, then connect in Step 10.

Wait for:

```
Info : [rp2350.dap.core0] Examination succeed
Startup breakpoint at 0x10000234 (2-byte hardware execute, one-shot).
Info : starting gdb server for rp2350.dap.core0 on 3333
Info : Listening on port 3333 for gdb connections
```

> **`BP_ADDR` parks the core at `main` before any client connects.** The script arms a 2-byte hardware breakpoint and then does the startup `reset run`, so the core runs from the vector table and stops at your address with no debugger attached yet. When IDA connects a moment later, the first thing it reads is already the truth: `Stopped at 0x10000234`. This is the whole reason the lab works cleanly — you never have to drive a reset from outside the GUI.
>
> Use the address you actually want to stop at:
>
> | What you want to stop at | Address | Command |
> | --- | --- | --- |
> | `main` (once per reset) | `0x10000234` | `BP_ADDR=0x10000234 ./debug-server.sh` |
> | The **loop** — the `FAV_NUM` `printf` call, hit every iteration | `0x10000292` | `BP_ADDR=0x10000292 ./debug-server.sh` |
>
> ```bash
> BP_ADDR=0x10000234 ./debug-server.sh   # park at main
> BP_ADDR=0x10000292 ./debug-server.sh   # park in the loop instead
> ```
>
> ```powershell
> $env:BP_ADDR="0x10000234"; .\debug-server.ps1   # park at main
> $env:BP_ADDR="0x10000292"; .\debug-server.ps1   # park in the loop
> ```
>
> **This startup stop is single-use.** OpenOCD flushes breakpoints when a client connects, so this one is gone once IDA attaches — fine for `main`, which only runs once per reset. Every breakpoint after that is set from the IDA GUI (Step 12) and is repeatable. To stop at `main` again, restart the server with `BP_ADDR` and reconnect.

> **Exactly one core.** The line must say `core0` and must **not** mention `core1`. Core1 is never started by this firmware; exposing it makes IDA read core1's reset-state registers, which are not real addresses, and OpenOCD floods the log with `Failed to read memory at 0xf0000000`. The scripts already use `USE_CORE=0`; do not change it.

> **Windows driver note:** the Debug Probe must use the **WinUSB** driver. If OpenOCD reports `unable to open CMSIS-DAP device`, install it with [Zadig](https://zadig.akeo.ie/) (select `Debug Probe (CMSIS-DAP)` -> WinUSB).

### Step 10: Connect IDA to the GDB server

1. Make sure the image is open and analyzed (Part 2) and the server from Step 9 is running (parked at `main`).
2. Choose `Debugger -> Connect to Remote Process`.
3. In the **adapter** dropdown, select **GDB MI**.
4. In the **connect** settings group, set **IP Address** to `127.0.0.1` and **Port** to `3333`.
5. Set **Full GDB Executable Path** to the `arm-none-eabi-gdb` from the **Arm GNU Toolchain 14.2.rel1**. It ships for all three hosts, and the Raspberry Pi Pico VS Code extension installs that same 14.2.rel1 toolchain (including `arm-none-eabi-gdb`) on all of them:

   | OS | `arm-none-eabi-gdb` path |
   | -- | ------------------------ |
   | macOS Apple Silicon | `/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin/arm-none-eabi-gdb` (or the Pico extension's `~/.pico-sdk/toolchain/14_2_Rel1/bin/arm-none-eabi-gdb`) |
   | Windows x64 | `%USERPROFILE%\.pico-sdk\toolchain\14_2_Rel1\bin\arm-none-eabi-gdb.exe` (Pico extension), or `C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin\arm-none-eabi-gdb.exe` |
   | Linux x64 | `~/.pico-sdk/toolchain/14_2_Rel1/bin/arm-none-eabi-gdb` (Pico extension), or the `bin/` directory of the extracted `arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi` tarball |
6. Click **Accept**.

> **Use the GDB MI adapter.** It launches a real `arm-none-eabi-gdb --interpreter=mi2` and lets IDA drive it, so breakpoints and stepping go through real GDB — which sends the correct 2-byte breakpoint length and handles step-over itself. Verified working end to end: connect, GUI breakpoints (**Add Hardware Breakpoint...**, hardware execute), **Step Into** / **Step Over**, and register edits. Stops are reported as `Breakpoint` (not `SingleStep`).
>
> **Do NOT have any breakpoints set in IDA before you connect.** With the GDB MI adapter, attaching while IDA already has a breakpoint **hangs the session**. Start the server parked with `BP_ADDR` (Step 9), connect, and only add hardware breakpoints *after* the connection is up. This is a IDA bug; it is the single most common GDB MI failure.
>
> **The GDB executable path matters.** Use the **14.2.rel1** build on every OS (Windows, macOS, Linux). The 13.3.rel1 build did **not** connect in testing.
>
> **This step is temporary.** Vector35 plans to ship a GDB binary with the GDB MI adapter ([Vector35/debugger#929](https://github.com/Vector35/debugger/issues/929), milestone *Langara*). Once that lands, IDA provides GDB itself and you will not need to set **Full GDB Executable Path** at all.
>
> **Do not pick Corellium.** IDA's adapter dropdown also lists **Corellium**, which is for Corellium's virtual devices and expects an API token, not a local OpenOCD server. It is not the adapter for this lab. The dropdown is a combo box, so an accidental arrow-key press can land on it — always read the label back and confirm it says **GDB MI** before clicking **Accept**.

> **The adapter and port are not saved in the `.i64`.** Every time you relaunch IDA you must re-select **GDB MI**, re-enter port `3333`, and re-set the GDB path.

> **Watch for an off-screen error dialog.** When a connection fails, IDA pops a `IDA critical alert` window that can be positioned mostly outside the main window, which makes it look like nothing happened. If the connect seems to do nothing, check your other display.

The target keeps running. Open the **Registers** tab (bug icon) and confirm you see live values. `pc` inside `0x10003xxx` and `sp` just below `0x20082000` are healthy.

> **If `pc` is `0x00000088`, `0x000000ec`, or `sp` is `0xf0000000`, the session is bad.** Restart the server, then restart IDA (a server restart while attached leaves IDA in a stale session), and connect again.

### Step 11: Find `main` without relying on its address

`main` can move between programs, so we do not guess it. We follow the one fixed path to it. Press `G` and go to `0x10000000`:

```
0x10000000   0x20082000   initial stack pointer (top of SRAM)
0x10000004   0x1000015d   reset vector
```

Bit 0 of a vector is the Thumb bit, so `0x1000015d` means "start at `0x1000015c`". That is `_reset_handler`. Follow the reset path to `0x10000186`, `platform_entry`:

```asm
10000186: ldr  r1, [pc, #80]  @ (100001d8 <data_cpy_table+0x38>)
10000188: blx  r1
1000018a: ldr  r1, [pc, #80]  @ (100001dc <data_cpy_table+0x3c>)
1000018c: blx  r1
1000018e: ldr  r1, [pc, #80]  @ (100001e0 <data_cpy_table+0x40>)
10000190: blx  r1
10000192: bkpt 0x0000
10000194: b.n  10000192  @ <platform_entry+0xc>
```

**The middle `blx` at `0x1000018c` is the call to `main`.** `platform_entry` is byte-identical in every project, so `0x1000018c` catches `main` no matter where the linker placed it. The literal pool at `0x100001dc` holds `main | 1`; clearing bit 0 gives `0x10000234`.

### Step 12: Set a hardware breakpoint in the GUI

With the **GDB MI** adapter, IDA sets breakpoints through real GDB, which sends the correct 2-byte length, so you set them **in the UI**. There is no command port here.

> **Why older drafts used the command port.** IDA's **GDB RSP** adapter is its own minimal RSP client and sends a **1-byte** breakpoint (`Z0,<addr>,1`); the Cortex-M33 FPB comparators need 2 bytes, so OpenOCD rejected it with `only breakpoints of two bytes length supported`. The old workaround was to arm breakpoints by hand over telnet. **The GDB MI adapter does not have this problem** — it drives real `arm-none-eabi-gdb`, which sends the right length. So everything below is done in the GUI. The command port still exists as a fallback (see the end of this step), but you do not need it.

#### Where you can stop

| You want to stop at | Address | How | Repeatable? |
| --- | --- | --- | --- |
| **`main`** | `0x10000234` | The server starts parked there with `BP_ADDR=0x10000234` (Step 9), so IDA is already stopped at `main` when it connects. | No — `main` runs once per reset. |
| **The `FAV_NUM` `printf` call** | `0x10000292` | Set a hardware breakpoint in the GUI, then click **Resume**. | Yes — fires on every iteration. |
| **The `OTHER_FAV_NUM` `printf` call** | `0x1000029c` | Set a hardware breakpoint in the GUI, then click **Resume**. | Yes — fires on every iteration. |
| **The `lcd_puts("Reverse")` call** | `0x1000027c` | Set a hardware breakpoint while stopped at `main`, then click **Resume**. | No — the LCD is written once at init. |

#### Set the loop breakpoint in the GUI

1. Press `G`, type the loop address (`0x10000292`), and press Enter.
2. Set a **hardware execution** breakpoint at that address, either way:
   - `Debugger -> Add Hardware Breakpoint...` — a **hardware execute** (`HE`) breakpoint. **Use this one.**
   - click the line and press `F2` (`Debugger -> Toggle Breakpoint`) — a **software** breakpoint. It will **not** work here: the code is in read-only flash, so GDB cannot install it and the core just keeps running.
3. Click **Resume**. The core is already running the loop, so the breakpoint fires on the next iteration. IDA stops with the PC at the loop address and reports it as a **Breakpoint** — verified: `Stopped (Breakpoint) at 0x10000292`.

> **No breakpoints before you connect.** With GDB MI, a breakpoint set before the connection hangs the session (Step 10). Start parked with `BP_ADDR`, connect, *then* add breakpoints.

#### Stepping

With the target halted at the breakpoint, **Step Into** (`F7`) and **Step Over** (`F8`) run through real GDB and move the PC. Verified: `0x10000292 -> 0x1000398c -> 0x1000398e -> ...`.

> **Step Over on the raw `.bin` steps *into* calls.** The raw image has no symbol for `__wrap_printf`, so **Step Over** at the `printf` call behaves like **Step Into**. When the lab needs to execute the call and then stop, it moves the breakpoint to the return site and clicks **Resume** instead (Step 13 shows this).

> **Never use IDA's Restart button.** On RP2350 it resets and halts inside the boot ROM (`pc=0x88`, `sp=0xf0000000`). To reset cleanly, restart the server with `BP_ADDR` and reconnect.

> **If you ever need the command port.** It is still there — `nc 127.0.0.1 4444`, and `bp <addr> 2 hw` still arms a breakpoint, `rbp <addr>` / `rbp all` still remove them. It is the fallback if you switch back to the **GDB RSP** adapter, whose 1-byte breakpoints the GUI cannot set. With GDB MI you do not need it for this lab.

### Step 13: HACK IT LIVE — change the printed `FAV_NUM`

`main` loads the `#define` `0x2a` (42) into `r1` and calls `printf` on every iteration. We break on that call and change it live:

```asm
1000028e: movs  r1, #42      @ 0x2a
10000290: ldr   r0, [pc, #32] @ (100002b4 <main+0x80>)
10000292: bl    1000398c     @ <__wrap_printf>
10000296: movw  r1, #1337    @ 0x539
1000029a: ldr   r0, [pc, #28] @ (100002b8 <main+0x84>)
1000029c: bl    1000398c     @ <__wrap_printf>
100002a0: b.n   1000028e     @ <main+0x5a>
```

1. Press `G`, go to `0x10000292` (the `bl __wrap_printf` for `FAV_NUM`).
2. Set a **hardware execute** breakpoint there: `Debugger -> Add Hardware Breakpoint...`. (Do not use `F2` — that is a software breakpoint and will not work on read-only flash.)
3. Click **Resume** in IDA. The target is already running the loop, so the breakpoint fires on the next iteration. IDA stops with the program counter at `0x10000292` and `r1 = 0x2a`.
4. Open the **Registers** widget (bug icon -> **Registers**).
5. Find `r1`. Its value is `0x2a` (42), loaded by the `movs r1, #42` at `0x1000028e`.
6. **Set `r1` to `0x2b` (43).** From IDA's Python console (`Plugins -> Python Console`):
   ```python
   dbg.set_reg_value("r1", 0x2b)
   ```
   `dbg.set_reg_value(name, value)` writes one register (returns `True` on success). You can also right-click `r1` in the **Registers** widget, press `E` (edit), type `2b`, and press Enter. The widget may not repaint the value, but the write reaches the target — you confirm it by the printed output in the next steps.
7. **Move the breakpoint past the call.** You want `printf` to run once and then stop, so move the breakpoint from `0x10000292` to the instruction *after* the call, `0x10000296` (the `movw r1, #1337` that begins the `OTHER_FAV_NUM` half of the loop): remove the breakpoint at `0x10000292` and set a hardware breakpoint at `0x10000296`. Two reasons not to just click **Step Over** here: a breakpoint left on the current PC re-traps the step, and IDA's **Step Over** steps *into* `__wrap_printf` on this raw `.bin` because the image carries no symbol for the call. Moving the breakpoint to the return site is deterministic.
8. Click **Resume** in IDA. The core executes `bl __wrap_printf` with `r1 = 0x2b`, so this iteration prints `FAV_NUM: 43`, then stops at `0x10000296`.
9. Look at your serial monitor — the `screen` session on the Pico's USB serial port — and at the **Target** tab in IDA:

   ```
   FAV_NUM: 43
   ```

You changed a running program's output without touching the binary.

### Step 13b: HACK THE LCD STRING LIVE — change `"Reverse"` to `"Exploit"` (optional)

The LCD text `"Reverse"` lives in flash (`.rodata`) at `0x10003ee8`, and flash is **read-only at runtime** — a debugger write there does not stick. So instead of overwriting the text in place, redirect the pointer: at the `lcd_puts` call for `"Reverse"`, `r0` holds the string address, so point `r0` at a replacement string you place in RAM.

> **Do this one while stopped at `main`, before `Resume`.** The LCD is written once during init, at `0x1000027c`. If you have already resumed into the loop, restart the server parked at `0x10000234` (Step 9) and reconnect, or the breakpoint at `0x1000027c` never fires again.

1. With IDA stopped at `main` (`0x10000234`), press `G` and go to `0x1000027c` (the `bl lcd_puts` that writes `"Reverse"`, loaded from the literal pool word at `0x100002ac`).
2. Set a **hardware execute** breakpoint at `0x1000027c` and click **Resume**. It fires once, with `r0 = 0x10003ee8`.
3. Put the replacement string into free RAM at `0x20080000` from IDA's **Python console** (`Plugins -> Python Console`) — no command port needed:
   ```python
   dbg.write_memory(0x20080000, b"Exploit\x00")
   ```
   `dbg.write_memory(address, bytes)` is IDA's debugger memory-write API; it returns `True` on success. That writes `Exploit\0`.
4. Point `r0` at that string:
   ```python
   dbg.set_reg_value("r0", 0x20080000)
   ```
   (Or right-click `r0` in the **Registers** widget, press `E`, type `0x20080000`, and press Enter.)
5. Move the breakpoint off the current PC (remove it at `0x1000027c`, set one at `0x10000280`, the `movs r0, #1` after the call) and click **Resume**. `lcd_puts` walks your RAM string and pushes `E x p l o i t` to the PCF8574 over I2C, so line 1 of the LCD now reads:

   ```
   Exploit
   ```

Like the value hack, this is **one boot only**: the next `lcd_puts` for `"Engineering"` is unaffected, but a reset reloads `r0` from flash. The permanent version is the static patch in Step 18c.

### Step 14: Why the hack reverts (and why we patch next)

Press **Resume**. The loop branches back to `0x1000028e`, which reloads `movs r1, #42`, so the next line is `FAV_NUM: 42`. The live edit changed one iteration only. There is no memory variable to change; the value is baked into the instruction. To make `FAV_NUM: 43` permanent we must patch the instruction. That is the static pass.

Press **Pause** to stop the output flood.

### Step 15: Kill the debugger and OpenOCD

The live hack is done. Do this **before** the static pass.

1. In the **Debugger** sidebar, click the **X** (**Kill**) (or **`Debugger -> Kill`**) to disconnect IDA.
2. **Kill does not stop the OpenOCD process** — `debug-server.sh` started it separately, and it keeps running and holding the probe. Stop it from the IDA console:

   **macOS / Linux:**

   ```python
   import subprocess
   subprocess.run(["pkill", "-TERM", "-f", "openocd"])  # stop the debug server, free the probe
   ```

   **Windows:**

   ```python
   import subprocess
   subprocess.run(["taskkill", "/F", "/IM", "openocd.exe"])  # stop the debug server, free the probe
   ```

3. Confirm nothing is left: `pgrep -fl openocd` (macOS/Linux) prints nothing.

From a terminal it is the same: `pkill -TERM -f openocd`, or `Get-Process openocd | Stop-Process` on Windows.

---

## Part 4: Static — Resolve the Functions in IDA and Patch

### Step 16: Resolve the functions in the IDA GUI

We now name the functions in IDA using the ELF symbol map from Step 4. IDA loaded the raw `.bin` with **no symbols**, so every function shows as `sub_<addr>` — resolving means giving each one its real name and signature.

Three keys do all the work:

| Key | IDA action | Use it for |
| --- | ------------------- | ---------- |
| `G` | Go to address | Jump to a function's address |
| `Y` | **Change Type** | Set the function's signature. The dialog shows the full prototype, so this sets the name *and* the type in one step. |
| `N` | Rename | Rename only, when you just want the name and not the type |

For each function below: `G` to its address, then **`Y` (Change Type)** and type the prototype from the table.

#### How to resolve a function in IDA (`Y`)

`Y` is the **Change Type** key, and it is what actually resolves the function — it turns `void sub_100037fc()` into `bool stdio_init_all(void)`. The Change Type dialog shows the full declaration (name and type), so typing the prototype sets both:

1. `G` to the function's address. The cursor lands on the function.
2. Press **`Y`**. In the Change Type dialog, type the prototype from the table exactly — for example `bool stdio_init_all(void)` — and press Enter.

The decompiler header then shows the real prototype, and calls to the function read cleanly instead of `sub_<addr>()`. `N` is only for renaming without touching the type; `Y` alone sets both the name and the type.

If `Y` seems to do nothing, confirm the cursor is on the function, or right-click it and pick **Change Type...**. IDA parses what you type and silently keeps the old type if it does not parse, so glance at the header after each `Y`.

#### Worked example: `main`

1. Press `G`, type `0x10000234`, press Enter. The view jumps there; the cursor lands on `sub_10000234`.
2. Press **`Y`** (Change Type), type `int main(void)`, press Enter. That sets the name to `main` and the type to `int(void)`.

> **IDA shows `int32_t` where Ghidra shows `int`.** After you set `int main(void)`, the decompiler header may read `int32_t main(void)`. That is the same type — on this platform `int` is 32 bits and IDA's parser normalises it to `int32_t`. Do not fight it; it is not an error.

#### Worked example: `i2c_init`

1. `G` -> `0x10003cdc`.
2. `Y` -> `uint i2c_init(i2c_inst_t* i2c, uint baudrate)`.

> **`i2c_init` returns `uint`, not `void`.** The SDK's `i2c_init` returns the actual configured baud rate; the ELF says `unsigned int (i2c_inst_t *, uint)`. Keep the return type.

#### Worked example: `lcd_i2c_init`

1. `G` -> `0x100002bc`.
2. `Y` -> `void lcd_i2c_init(i2c_inst_t* i2c, uint8_t pcf_addr, int nibble_shift, uint8_t backlight_mask)`.

This is our own `lcd_1602.c` code. In this build it is one big function: the compiler inlined `lcd_store_config`, `lcd_hd44780_reset`, and `lcd_hd44780_configure` into it.

#### Worked example: `lcd_set_cursor`

1. `G` -> `0x100006f4`.
2. `Y` -> `void lcd_set_cursor(int line, int position)`.

#### Worked example: `lcd_puts`

1. `G` -> `0x100007f0`.
2. `Y` -> `void lcd_puts(const char* s)`.

#### Worked example: `gpio_set_function`

1. `G` -> `0x100008f0`.
2. `Y` -> `void gpio_set_function(uint gpio, gpio_function_t fn)`.

#### Worked example: `gpio_set_pulls`

1. `G` -> `0x1000092c`.
2. `Y` -> `void gpio_set_pulls(uint gpio, bool up, bool down)`.

This is what `gpio_pull_up(2)` in our source compiles to: the SDK's `static inline` `gpio_pull_up` disappears, and `main` calls `gpio_set_pulls(2, true, false)` directly at `0x10000258`.

#### Worked example: `__wrap_printf`

1. `G` -> `0x1000398c`.
2. `Y` -> `int __wrap_printf(const char *fmt, ...)`. Keep the `...` — `printf` is variadic.

#### Worked example: `stdio_init_all`

1. `G` -> `0x100037fc`.
2. `Y` -> `bool stdio_init_all(void)`.

> **`printf` in our source is `__wrap_printf` in the binary.** The SDK links our `printf` calls to its `__wrap_printf` wrapper, which forwards to `__wrap_vprintf`. Rename it `printf` if you prefer the lesson's shorthand, but `__wrap_printf` is what the ELF says.

The rest of the chain is the same two keystrokes per function (`G`, then `Y`). This is **our code plus the library functions it actually calls** — not the whole SDK. `main` calls `stdio_init_all`, `i2c_init`, `gpio_set_function`, `gpio_set_pulls`, `lcd_i2c_init`, `lcd_set_cursor`, `lcd_puts`, and `printf`, so we follow that chain down.

The call chain for this project:

```
main
├── stdio_init_all ── stdio_uart_init ── gpio_set_function, uart_init, stdio_set_driver_enabled
│                                        └── uart_init ── clock_get_hz, busy_wait_us
├── i2c_init
├── gpio_set_function
├── gpio_set_pulls                 (gpio_pull_up inlined)
├── lcd_i2c_init ── i2c_write_blocking, sleep_us, sleep_ms
│   └── pcf_write_byte / pcf_pulse_enable / lcd_write4 / lcd_send / lcd_store_config /
│       lcd_hd44780_reset / lcd_hd44780_configure   (all inlined; no calls)
├── lcd_set_cursor ── i2c_write_blocking, sleep_us
├── lcd_puts ── i2c_write_blocking, sleep_us
└── __wrap_printf ── __wrap_vprintf ── vfctprintf ── _vsnprintf
    │                                              └── _ntoa_format / _out_rev / _out_char
    ├── time_us_64
    └── stdio_out_chars_crlf
```

**Resolve every function in that chain:**

| Address | Rename to (`N`) | Signature (`Y`) |
| ------- | --------------- | --------------- |
| `0x1000015c` | `_reset_handler` | `void _reset_handler(void)` |
| `0x10000186` | `platform_entry` | `void platform_entry(void)` |
| `0x1000019a` | `data_cpy` | `void data_cpy(void*, void*, void*)` |
| `0x100001e4` | `_init` | `void _init(void)` |
| `0x10000210` | `frame_dummy` | `void frame_dummy(void)` |
| **`0x10000234`** | **`main`** | **`int main(void)`** |
| `0x100002bc` | `lcd_i2c_init` | `void lcd_i2c_init(i2c_inst_t*, uint8_t, int, uint8_t)` |
| `0x100006f4` | `lcd_set_cursor` | `void lcd_set_cursor(int, int)` |
| `0x100007f0` | `lcd_puts` | `void lcd_puts(const char*)` |
| `0x100008f0` | `gpio_set_function` | `void gpio_set_function(uint, gpio_function_t)` |
| `0x1000092c` | `gpio_set_pulls` | `void gpio_set_pulls(uint, bool, bool)` |
| `0x10001368` | `sleep_us` | `void sleep_us(uint64_t)` |
| `0x10001440` | `sleep_ms` | `void sleep_ms(uint32_t)` |
| `0x10001624` | `time_us_64` | `uint64_t time_us_64(void)` |
| `0x10001638` | `busy_wait_us` | `void busy_wait_us(uint64_t)` |
| `0x100016b8` | `uart_init` | `uint uart_init(uart_inst_t*, uint)` |
| `0x1000188c` | `clock_get_hz` | `unsigned long clock_get_hz(clock_handle_t)` |
| `0x100035a4` | `exit` | `void exit(int)` |
| `0x100035ac` | `runtime_init` | `void runtime_init(void)` |
| `0x100035d8` | `stdio_out_chars_crlf` | `void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)` |
| `0x100036e8` | `stdio_put_string` | `int stdio_put_string(const char*, int, bool, bool)` |
| `0x100037d4` | `stdio_set_driver_enabled` | `void stdio_set_driver_enabled(stdio_driver_t*, bool)` |
| `0x100037fc` | `stdio_init_all` | `bool stdio_init_all(void)` |
| `0x100038c8` | `__wrap_vprintf` | `int __wrap_vprintf(const char*, va_list)` |
| `0x1000398c` | `__wrap_printf` | `int __wrap_printf(const char*, ...)` |
| `0x10003b48` | `stdio_uart_init` | `void stdio_uart_init(void)` |
| `0x10003cdc` | `i2c_init` | `uint i2c_init(i2c_inst_t*, uint)` |
| `0x10003d28` | `i2c_write_blocking` | `int i2c_write_blocking(i2c_inst_t*, uint8_t, const uint8_t*, size_t, bool)` |
| `0x10003e24` | `strlen` | `size_t strlen(const char*)` |

**Resolve the SDK helpers the `printf` path reaches:**

| Address | Rename to (`N`) | Signature (`Y`) |
| ------- | --------------- | --------------- |
| `0x10003548` | `vfctprintf` | `int vfctprintf(void (*)(char, void*), void*, const char*, va_list)` |
| `0x10002b64` | `_vsnprintf` | `int _vsnprintf(out_fct_type, char*, size_t, const char*, va_list)` |
| `0x10001f48` | `_ntoa_format` | `unsigned _ntoa_format(out_fct_type, char*, size_t, size_t, char*, size_t, bool, unsigned, unsigned, unsigned, unsigned)` |
| `0x10001eac` | `_out_rev` | `unsigned _out_rev(out_fct_type, char*, size_t, size_t, const char*, size_t, unsigned, unsigned)` |
| `0x1000211c` | `_out_char` | `void _out_char(char, void*, size_t, size_t)` |

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

> **`i2c1_inst` is data, not a function.** `arm-none-eabi-nm -n` lists `2000062c T i2c1_inst`. The `T` is a **global data** symbol: the linker parks the `i2c1_inst` struct at RAM address `0x2000062c`. Its first word is the hardware pointer `0x40098000`, the I2C1 register base. There is no function there; do not `Y` it with a prototype.

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
> typedef void (*out_fct_type)(char, void*, size_t, size_t);
> struct stdio_driver;
> typedef struct stdio_driver stdio_driver_t;
> struct uart_inst;
> typedef struct uart_inst uart_inst_t;
> struct i2c_inst;
> typedef struct i2c_inst i2c_inst_t;
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
>     0x100002bc: ("lcd_i2c_init",              "void lcd_i2c_init(i2c_inst_t*, uint8_t, int, uint8_t)"),
>     0x100006f4: ("lcd_set_cursor",            "void lcd_set_cursor(int, int)"),
>     0x100007f0: ("lcd_puts",                  "void lcd_puts(const char*)"),
>     0x100008f0: ("gpio_set_function",         "void gpio_set_function(uint, gpio_function_t)"),
>     0x1000092c: ("gpio_set_pulls",            "void gpio_set_pulls(uint, bool, bool)"),
>     0x10001368: ("sleep_us",                  "void sleep_us(uint64_t)"),
>     0x10001440: ("sleep_ms",                  "void sleep_ms(uint32_t)"),
>     0x10001624: ("time_us_64",                "uint64_t time_us_64(void)"),
>     0x10001638: ("busy_wait_us",              "void busy_wait_us(uint64_t)"),
>     0x100016b8: ("uart_init",                 "uint uart_init(uart_inst_t*, uint)"),
>     0x1000188c: ("clock_get_hz",              "unsigned long clock_get_hz(clock_handle_t)"),
>     0x100035a4: ("exit",                      "void exit(int)"),
>     0x100035ac: ("runtime_init",              "void runtime_init(void)"),
>     0x100035d8: ("stdio_out_chars_crlf",      "void stdio_out_chars_crlf(stdio_driver_t*, const char*, int)"),
>     0x100036e8: ("stdio_put_string",          "int stdio_put_string(const char*, int, bool, bool)"),
>     0x100037d4: ("stdio_set_driver_enabled",  "void stdio_set_driver_enabled(stdio_driver_t*, bool)"),
>     0x100037fc: ("stdio_init_all",            "bool stdio_init_all(void)"),
>     0x100038c8: ("__wrap_vprintf",            "int __wrap_vprintf(const char*, va_list)"),
>     0x1000398c: ("__wrap_printf",             "int __wrap_printf(const char*, ...)"),
>     0x10003b48: ("stdio_uart_init",           "void stdio_uart_init(void)"),
>     0x10003cdc: ("i2c_init",                  "uint i2c_init(i2c_inst_t*, uint)"),
>     0x10003d28: ("i2c_write_blocking",        "int i2c_write_blocking(i2c_inst_t*, uint8_t, const uint8_t*, size_t, bool)"),
>     0x10003e24: ("strlen",                    "size_t strlen(const char*)"),
>     0x10003548: ("vfctprintf",                "int vfctprintf(void (*)(char, void*), void*, const char*, va_list)"),
>     0x10002b64: ("_vsnprintf",                "int _vsnprintf(out_fct_type, char*, size_t, const char*, va_list)"),
>     0x10001f48: ("_ntoa_format",              "unsigned _ntoa_format(out_fct_type, char*, size_t, size_t, char*, size_t, bool, unsigned, unsigned, unsigned, unsigned)"),
>     0x10001eac: ("_out_rev",                  "unsigned _out_rev(out_fct_type, char*, size_t, size_t, const char*, size_t, unsigned, unsigned)"),
>     0x1000211c: ("_out_char",                 "void _out_char(char, void*, size_t, size_t)"),
> }
> for addr, (name, sig) in funcs.items():
>     bv.define_user_symbol(Symbol(SymbolType.FunctionSymbol, addr, name))
>     f = bv.get_function_at(addr)
>     if f is not None:
>         f.set_user_type(sig)
> ```
>
> SDK type names (`i2c_inst_t`, `uart_inst_t`, `stdio_driver_t`, `gpio_function_t`, plus `uint`, `va_list`, `clock_handle_t`, and `out_fct_type`) are **not** in the raw `.bin`. `set_user_type` re-parses each signature as C, so an undefined name raises `SyntaxError: unknown type name '...'` and stops the loop — it is not harmless. The `sdk` block above defines them first (an opaque `struct`/`enum`/`typedef` is enough to parse). If you add a function that uses another SDK type, add a definition for it to that block too.

### Step 17: Read `main` in the decompiler

Open the **Decompiler** view on `main`. Once the functions above are typed, it reads roughly:

```c
int32_t main(void)
{
    stdio_init_all();
    i2c_init(&i2c1_inst, 0x186a0);              // i2c_init(i2c1, 100000)
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_set_pulls(2, true, false);             // gpio_pull_up(2)
    gpio_set_pulls(3, true, false);             // gpio_pull_up(3)
    lcd_i2c_init(&i2c1_inst, 0x27, 4, 8);       // lcd_i2c_init(i2c1, 0x27, 4, 0x08)
    lcd_set_cursor(0, 0);
    lcd_puts("Reverse");
    lcd_set_cursor(1, 0);
    lcd_puts("Engineering");
    while (true) {
        __wrap_printf("FAV_NUM: %d\r\n", 0x2a);         // FAV_NUM  = 42
        __wrap_printf("OTHER_FAV_NUM: %d\r\n", 0x539);  // OTHER_FAV_NUM = 1337
    }
}
```

The `0x2a` and `0x539` are the constants we will patch. Both are **immediates in the instruction stream** — there is no `.rodata` word to change, which is why the patch edits the instruction operand. Now make the hacks permanent.

### Step 18: Patch 1 — change `FAV_NUM` from 42 to 43

Go to `0x1000028e`:

```asm
1000028e: 2a 21   movs  r1, #42   @ 0x2a
```

The halfword is `0x212a`, stored little-endian as `2a 21`. The immediate is the low byte, so the byte at the instruction's own address is `0x2a`. Change it to `0x2b` (43).

| Project | Address | Before | After | Effect |
| ------- | ------- | ------ | ----- | ------ |
| `0x0017` | `0x1000028e` | `2a` | `2b` | `movs r1, #42` -> `#43`, prints `FAV_NUM: 43` |

**Option A — Hex view:**

1. Switch to the **Hex** view (`View -> Hex`).
2. Toggle the lock off so editing is enabled.
3. Go to `0x1000028e` and change the byte `2A` to `2B`.
4. Return to the linear view, right-click the function -> `Reanalyze`.

**Option B — Python console:**

```python
bv.write(0x1000028e, b"\x2b")
print(hex(bv.read(0x1000028e, 1)[0]))  # -> 0x2b
```

After reanalysis the instruction reads `movs r1, #43`.

### Step 18b: Patch 2 — change `OTHER_FAV_NUM` from 1337 to 1344

Go to `0x10000296`:

```asm
10000296: 40 f2 39 51   movw  r1, #1337   @ 0x539
```

This is the 32-bit Thumb-2 encoding of `movw r1, #0x539`. The four bytes and their roles:

```
+-----------------------------------------------------------------+
|  movw r1, #0x539  ->  bytes: 40 F2 39 51                        |
|                                                                 |
|  Byte 0: 0x40  -+                                               |
|  Byte 1: 0xF2  -+   First halfword (opcode + upper imm bits)    |
|  Byte 2: 0x39  ---- Lower 8 bits of immediate (imm8) <- CHANGE  |
|  Byte 3: 0x51  ---- Destination register (r1) + upper imm bits  |
|                                                                 |
|  imm16 = 0x0539 = 1337 decimal                                  |
|  imm8 field = 0x39 (lower 8 bits of the value)                  |
|                                                                 |
+-----------------------------------------------------------------+
```

The imm8 byte is the **third** byte of the 4-byte instruction: the instruction starts at `0x10000296`, so the byte to change is `0x10000296 + 2 = 0x10000298`. Change `0x39` to `0x40`, which changes the value from `0x539` (1337) to `0x540` (1344).

| Project | Address | Before | After | Effect |
| ------- | ------- | ------ | ----- | ------ |
| `0x0017` | `0x10000298` | `39` | `40` | `movw r1, #1337` -> `#1344`, prints `OTHER_FAV_NUM: 1344` |

**Option A — Hex view:**

1. Switch to the **Hex** view (`View -> Hex`).
2. Go to `0x10000298` and change the byte `39` to `40`.
3. Return to the linear view and reanalyze.

**Option B — Python console:**

```python
bv.write(0x10000298, b"\x40")
print(bv.read(0x10000296, 4).hex())  # -> 40f24051
```

> **Do not patch `0x10000296` itself.** That is the instruction's first byte (the opcode), not the immediate. The immediate's low 8 bits are at `0x10000298`; patching the opcode corrupts the instruction.

### Step 18c: Patch 3 — change the LCD text from `"Reverse"` to `"Exploit"`

The string `"Reverse"` starts at `0x10003ee8`. Its eight bytes are `52 65 76 65 72 73 65 00` (`Reverse\0`). Change them to `45 78 70 6c 6f 69 74 00` (`Exploit\0`). **Both strings are exactly seven characters**, so the replacement fits without touching `"Engineering"` at `0x10003ef0`.

| Project | Address | Before | After | Effect |
| ------- | ------- | ------ | ----- | ------ |
| `0x0017` | `0x10003ee8` | `52 65 76 65 72 73 65 00` | `45 78 70 6c 6f 69 74 00` | LCD line 1 prints `Exploit` instead of `Reverse` |

**ASCII reference:**

| Character | Hex |
| --------- | --- |
| E | `0x45` |
| x | `0x78` |
| p | `0x70` |
| l | `0x6c` |
| o | `0x6f` |
| i | `0x69` |
| t | `0x74` |

**Option A — Hex view:**

1. Switch to the **Hex** view (`View -> Hex`).
2. Go to `0x10003ee8` and change the eight bytes `52 65 76 65 72 73 65 00` to `45 78 70 6c 6f 69 74 00`.
3. Return to the linear view and reanalyze.

**Option B — Python console:**

```python
bv.write(0x10003ee8, b"Exploit\x00")
print(bv.read(0x10003ee8, 8))  # -> b'Exploit\x00'
```

Keep the replacement exactly eight bytes. If you use a shorter string you must pad it and keep the terminating `\0`, or `lcd_puts` will run into the `"Engineering"` string that follows at `0x10003ef0`.

### Step 19: Export the patched `.bin`

```python
import os
seg = next(s for s in bv.segments if s.data_length)  # the loadable image segment
data = bv.read(seg.start, seg.data_length)  # base + size come from the view itself
out = os.path.join(os.path.join(root, "0x0017_constants", "build"), "0x0017_constants-h.bin")
open(out, "wb").write(data)
print(len(data), out)  # -> 17980 /.../build/0x0017_constants-h.bin
```

Where the two numbers come from — nothing is hardcoded:

- **`seg.start`** is the image base IDA loaded the `.bin` at (`0x10000000`), the same value you pass to `uf2conv --base`.
- **`seg.data_length`** is the segment's size in the file (`0x463c` = 17980). Exactly one segment carries data (the image); every peripheral and synthetic segment has `data_length == 0`, so `next(...)` picks the image.
- Reading `seg.start` for `seg.data_length` bytes therefore grabs exactly the image.

Two gotchas this avoids:

- **No relative path.** IDA's Python console runs with a read-only working directory (inside the app bundle), so `open("0x0017_constants-h.bin", "wb")` fails with `OSError: [Errno 30] Read-only file system`. `root` (from `~/.embedded-hacking-repo`, Step 3) is the repo, so the file is written into the project's `build/` — no machine-specific path and no database needed.
- **Read the image, not the whole view.** `bv.read(bv.start, bv.length)` spans the entire mapped range, which is not the image. The segment's `data_length` is the image size.

A different size means you exported a partial view.

### Step 20: Convert to UF2

Run from the project directory:

**macOS Apple Silicon / Linux x64:**

```bash
python3 ../uf2conv.py 0x0017_constants-h.bin \
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

**Windows x64:**

```cmd
python ..\uf2conv.py 0x0017_constants-h.bin ^
  --base 0x10000000 --family 0xe48bff59 --output hacked.uf2
```

> **Or convert from the IDA console** — it is a normal Python interpreter, so you never have to leave the app. `chdir` to a writable directory first (the default one is read-only), then run the script:
>
> ```python
> import os, sys, runpy
> os.chdir(os.path.join(root, "0x0017_constants", "build"))   # the project build dir (writable)
> sys.argv = ["uf2conv.py", "0x0017_constants-h.bin",
>             "--base", "0x10000000", "--family", "0xe48bff59", "--output", "hacked.uf2"]
> runpy.run_path("../../uf2conv.py", run_name="__main__")   # path to your uf2conv.py
> ```
>
> This writes `hacked.uf2` next to the `.bin`, ready to drag onto the Pico.

### Step 21: Flash and verify

Hold **BOOTSEL**, plug in the Pico 2, and drag `hacked.uf2` onto the **`RP2350`** drive. Open the serial monitor:

```
FAV_NUM: 43
OTHER_FAV_NUM: 1344
FAV_NUM: 43
OTHER_FAV_NUM: 1344
...
```

and the **LCD line 1 reads `Exploit`** while line 2 still reads `Engineering`.

**Both constants changed and the LCD string changed — with nine operand bytes patched and no source code.** (`0x2a -> 0x2b`, `0x39 -> 0x40`, and the eight-byte string.)

> **Faster: flash over the Debug Probe (no BOOTSEL).** The repo's `flash.sh` writes the raw `.bin` straight into XIP flash over SWD (`program <bin> 0x10000000 verify reset exit`), so you never touch BOOTSEL or a UF2. Run it from a terminal (`./flash.sh <bin>`), or from the IDA console **without freezing it** — use `subprocess.Popen`, which returns immediately, and send OpenOCD's output to a log file. (`subprocess.run` blocks the console until the flash finishes; do not use it here.)
>
> ```python
> import os, subprocess
> root = open(os.path.expanduser("~/.embedded-hacking-repo")).read().strip()
> bin_path = os.path.join(os.path.join(root, "0x0017_constants", "build"), "0x0017_constants-h.bin")
> log = os.path.join(os.path.join(root, "0x0017_constants", "build"), "flash.log")
> subprocess.run(["pkill", "-TERM", "-f", "openocd"])  # free the probe first
> p = subprocess.Popen([os.path.join(root, "flash.sh"), bin_path],
>                      stdout=open(log, "w"), stderr=subprocess.STDOUT, start_new_session=True)
> print("flashing in the background; log:", log)
> ```
>
> The `pkill` frees the probe first; on Windows use `subprocess.run(["taskkill", "/F", "/IM", "openocd.exe"])`.
>
> The console is free the moment this returns. Check it with `print(p.poll())` (`None` = still running, `0` = done) or read `flash.log` — success ends with `** Verified OK **`.
>
> The same non-blocking form without the script:
>
> ```python
> import os, subprocess
> ocd = os.path.expanduser("~/.pico-sdk/openocd/0.12.0+dev")
> bin_path = os.path.join(os.path.join(root, "0x0017_constants", "build"), "0x0017_constants-h.bin")
> log = os.path.join(os.path.join(root, "0x0017_constants", "build"), "flash.log")
> subprocess.run(["pkill", "-TERM", "-f", "openocd"])  # free the probe first
> p = subprocess.Popen([f"{ocd}/openocd", "-s", f"{ocd}/scripts",
>     "-f", "interface/cmsis-dap.cfg", "-f", "target/rp2350.cfg",
>     "-c", "adapter speed 5000",
>     "-c", f"program {bin_path} 0x10000000 verify reset exit"],
>     stdout=open(log, "w"), stderr=subprocess.STDOUT, start_new_session=True)
> print("flashing in the background; log:", log)
> ```
>
> **The Debug Probe is single-owner.** If IDA is still attached (the `debug-server.sh` OpenOCD is running), the flash cannot grab the probe. Detach in IDA and stop that OpenOCD first:
>
> ```bash
> # macOS / Linux
> pkill -TERM -f openocd
> ```
> ```powershell
> # Windows
> Get-Process openocd -ErrorAction SilentlyContinue | Stop-Process
> ```
>
> Success looks like `Programming Finished` -> `Verified OK` -> `Resetting Target`. On Windows use `flash.ps1` (`.\flash.ps1 -Bin <path>`) the same way.

---

## Cheatsheet

### IDA GUI actions

| Action | How |
| ------ | --- |
| Go to address | `G` |
| Rename function/symbol | `N` |
| Set type or signature | `Y` |
| Add comment | `;` |
| Open Hex view | `View -> Hex` |
| Enable hex editing | Toggle the lock in the status bar |
| Reanalyze after a patch | Right-click function -> `Reanalyze` |
| Edit a register live | `dbg.set_reg_value("r1", 0x2b)` in the Python console (or right-click the register, press `E`, type hex, Enter) |
| Write a RAM string live | `dbg.write_memory(0x20080000, b"Exploit\x00")` |
| Set a breakpoint | `Debugger -> Add Hardware Breakpoint...` (hardware execute). Do **not** use `F2` — software breakpoints cannot be written to read-only flash. |
| Move a breakpoint | Remove it and set it at the new address in the GUI (command-port fallback: `rbp <old addr>` then `bp <new addr> 2 hw`) |
| Confirm what is armed | The **Breakpoints** widget lists it (command-port fallback: `mdw 0xE0002000 8`, each armed breakpoint shows as `<addr \| 1>`) |
| Apply the ELF symbol map | Paste the Python snippet from Step 16 into the Python Console |

### OpenOCD server and reset

The server runs with `gdb_breakpoint_override hard` so that flash-writes are never attempted. Breakpoints in this lab are set in the IDA GUI through the **GDB MI** adapter (Step 12). The command-port rows below are the fallback if you use the **GDB RSP** adapter instead.

| Action | Command |
| ------ | ------- |
| Connect to the OpenOCD prompt (fallback) | `nc 127.0.0.1 4444` (or `telnet 127.0.0.1 4444`) |
| Reset and run (command port) | `reset run` |
| Check core state (command port) | `targets` |
| Set a breakpoint in the GUI | `Debugger -> Add Hardware Breakpoint...` (hardware execute; `F2` software breakpoints do not work on flash) |
| (fallback) Add a breakpoint without the GUI | `bp <addr> 2 hw` |
| Remove one breakpoint | `rbp <addr>` — **address only, no length, no `hw`** |
| Remove every breakpoint | `rbp all` |
| Start the server parked at `main` | macOS/Linux: `BP_ADDR=0x10000234 ./debug-server.sh` — Windows: `$env:BP_ADDR="0x10000234"; .\debug-server.ps1` (one-shot) |
| Start the server parked in the loop | macOS/Linux: `BP_ADDR=0x10000292 ./debug-server.sh` — Windows: `$env:BP_ADDR="0x10000292"; .\debug-server.ps1` |
| Break on the loop in a running target | set a hardware breakpoint in the GUI at the loop address, then **Resume** — repeatable |
| Make IDA stepping work | `rp2350.dap.core0 configure -rtos none` (already in the scripts) |
| Step without re-trapping | move the breakpoint off the current PC first, then **Step Into**/**Step Over** |
| Reset without desyncing IDA | **Detach**, `reset run` on the port, reconnect — never `reset run` while attached |

### Every address and byte we changed

| Project | Address | Before | After | Effect |
| ------- | ------- | ------ | ----- | ------ |
| `0x0017` | `0x1000028e` | `2a` | `2b` | `movs r1, #42` -> `#43`, prints `FAV_NUM: 43` |
| `0x0017` | `0x10000298` | `39` | `40` | `movw r1, #1337` -> `#1344`, prints `OTHER_FAV_NUM: 1344` |
| `0x0017` | `0x10003ee8` | `52 65 76 65 72 73 65 00` | `45 78 70 6c 6f 69 74 00` | LCD line 1 prints `Exploit` instead of `Reverse` |

### The I2C/LCD memory map

| Item | Address | Notes |
| ---- | ------- | ----- |
| `i2c1_inst` | `0x2000062c` | RAM `.data`, fixed for the life of the program |
| `i2c1_inst.hw` | `0x40098000` | I2C1 hardware register base (first word of the struct) |
| `i2c1_inst.restart_on_next` | `0x20000630` | second word of the struct, `0` (false) |
| Literal-pool word 1 | `0x100002a4` | `0x000186a0` — I2C baud rate, 100000 |
| Literal-pool word 2 | `0x100002a8` | `0x2000062c` — `&i2c1_inst` |
| Literal-pool word 3 | `0x100002ac` | `0x10003ee8` — pointer to `"Reverse"` |
| Literal-pool word 4 | `0x100002b0` | `0x10003ef0` — pointer to `"Engineering"` |
| Literal-pool word 5 | `0x100002b4` | `0x10003efc` — pointer to `"FAV_NUM: %d\r\n"` |
| Literal-pool word 6 | `0x100002b8` | `0x10003f0c` — pointer to `"OTHER_FAV_NUM: %d\r\n"` |

### Raw image facts

| Item | Value |
| ---- | ----- |
| Build type | `Release` |
| Load base address | `0x10000000` |
| Project size | `17980` bytes (`0x463c`) |
| Fixed `main` anchor | `0x1000018c` (reset handler middle `blx`) |
| `main` | `0x10000234` |
| `printf` call / return, `FAV_NUM` | `0x10000292` / `0x10000296` |
| `printf` call, `OTHER_FAV_NUM` | `0x1000029c` |
| `i2c1_inst` RAM address | `0x2000062c` |
| I2C1 hardware registers | `0x40098000` |
| `FAV_NUM` format string | `0x10003efc` |
| `OTHER_FAV_NUM` format string | `0x10003f0c` |
| `"Reverse"` string | `0x10003ee8` |
| `"Engineering"` string | `0x10003ef0` |
| RP2350 UF2 family ID | `0xe48bff59` |

---

## Troubleshooting

### IDA hangs or crashes when you connect (macOS 27)

Three different causes have been seen on this setup; check them in this order.

- **A breakpoint set before connecting.** With the **GDB MI** adapter, if the binary view already has a breakpoint, the session hangs. Start parked with `BP_ADDR`, connect, then add breakpoints (see the next entry).
- **The wrong GDB executable.** Point **Full GDB Executable Path** at the **14.2.rel1** toolchain (Step 10). The 13.3.rel1 build did **not** connect in testing.
- **The LLDB adapter.** A crash report with `libdebuggercore.dylib -> std::terminate() -> abort()` and `liblldb` in the stack is the **LLDB** adapter, not GDB MI. Avoid LLDB on this setup.

**Use GDB MI**, with the 14.2.rel1 path above. If it still fails, fall back to plain `arm-none-eabi-gdb` against the same server — the addresses and register values are identical to the GUI steps.

If IDA hangs, force-quit it; the connect dialog has no working Cancel. The static steps (resolve, patch, export, flash) never touch the debugger and always work.

### The GUI refuses to set a breakpoint (GDB RSP adapter only)

If you are on the **GDB RSP** adapter, the GUI cannot set breakpoints on this target. That adapter is IDA's own minimal RSP client and sends a **1-byte** breakpoint (`Z0,<addr>,1`); the Cortex-M33 comparators need 2 bytes, so OpenOCD answers `only breakpoints of two bytes length supported`. It affects every address, both `Toggle Breakpoint` and `Add Hardware Breakpoint`, and the dialog's **Size** field is disabled. `gdb_breakpoint_override` makes no difference.

**Fix: use the GDB MI adapter** (Step 10). It drives real GDB, which sends the correct length, so GUI breakpoints just work. If you must stay on GDB RSP, arm breakpoints from the command port after connecting (`bp <addr> 2 hw`) — but the lab uses GDB MI and does not need that.

### GDB MI hangs when you connect (a breakpoint already existed)

With the **GDB MI** adapter, if IDA already has a breakpoint set when you connect, the session **hangs**. This is a IDA bug. The working order is:

1. Start the server parked, e.g. `BP_ADDR=0x10000234 ./debug-server.sh` (Windows: `$env:BP_ADDR="0x10000234"; .\debug-server.ps1`).
2. Connect with the **GDB MI** adapter.
3. Only *then* set hardware breakpoints in the UI.

Never have a breakpoint in the binary view before the GDB MI connection. If it hangs, quit IDA, restart the server with `BP_ADDR`, and connect again before adding any breakpoints.

### Step Into / Step Over does nothing (PC never moves)

Two causes have been seen on this target.

1. **A breakpoint on the current PC re-traps the step.** OpenOCD's step-over-breakpoint logic fails with `Duplicate Breakpoint address` and the PC stays put. Fix: move the breakpoint off the current PC (in the GUI), then step.
2. **The `hwthread` RTOS (GDB RSP adapter only).** With the **GDB RSP** adapter, OpenOCD can log `fake step thread 0` and reply without stepping, because the RP2350 config's `-rtos hwthread` makes the current thread id 1 while IDA sends thread id 0. Fix: `rp2350.dap.core0 configure -rtos none` (the launcher scripts already pass this). **GDB MI does not hit this.**

To tell them apart, turn on OpenOCD logging (`log_output /tmp/ocd.log`, then `debug_level 3` on the command port) and look for `fake step` versus `Duplicate Breakpoint`.

### `zsh: bad CPU type in executable: cmake`

An Intel `x86_64` tool is on your `PATH` on Apple Silicon. Run Step 2: `export PATH="/opt/homebrew/bin:$PATH"`, then `hash -r`. Add it to `~/.zshrc` to make it permanent.

### My addresses do not match this guide

You probably built `Debug`. This lesson is a `Release` build. Re-run Step 3 with `-DCMAKE_BUILD_TYPE=Release`. A `Debug` build moves the SDK functions and keeps `init_i2c_and_lcd` / `write_lcd_greeting` as separate calls, so `main` is not at `0x10000234`.

### A breakpoint never fires

First, confirm you actually set one, and that it is a **hardware** breakpoint. With the **GDB MI** adapter, `Debugger -> Add Hardware Breakpoint...` (hardware execute) should land in the **Breakpoints** widget. If nothing lands, or the core keeps running, you probably used `F2` (`Toggle Breakpoint`) — that is a software breakpoint and cannot be written to read-only flash, so it never installs. Also check you are on **GDB MI**, not **GDB RSP** (the GDB RSP adapter cannot set breakpoints on this target at all).

Then check the order and the state:

- **Arm it only after IDA is connected.** OpenOCD flushes every breakpoint when a client attaches, so anything armed earlier is gone. This also applies to `BP_ADDR` on the startup command line.
- **Verify it is armed:** `mdw 0xE0002000 8`. You should see your address with the low bit set (`0x10000292` -> `0x10000293`). All zeros means nothing is armed — re-read this first, because it distinguishes "not armed" from "armed but never reached".
- **Is the core running?** `poll` on the command port should not report a halt. If it is stopped, click **Resume**.
- **Does the address get reached again?** `main` runs once per reset, so use `BP_ADDR` at startup (Step 9) rather than `reset run` while attached. Loop addresses such as `0x10000292` fire on the next pass with no reset — arm them and click **Resume** in IDA.
- **With GDB MI the stop is reported as `Breakpoint`** and appears in the **Breakpoints** widget, because GDB really did set it.

### I edit `r1` (or another register) and it reverts

`main` reloads the value at the top of every loop iteration — `movs r1, #42` at `0x1000028e` runs right before the `printf` at `0x10000292`. So `r1` is only `0x2b` for the instant between your edit and the next pass; then it is `0x2a` again. The edit sticks only if the core is **genuinely stopped** at the breakpoint and stays stopped.

If it keeps reverting, the core is running, which almost always means the breakpoint is not installed — usually because it is a **software** breakpoint (`F2`) that cannot be written to read-only flash. Use `Debugger -> Add Hardware Breakpoint...` (hardware execute).

> **The Registers widget is a snapshot, not a live view.** IDA reads the registers at each stop and shows that snapshot; it does not poll the target, and there is no "refresh registers" command. So a value changed outside IDA will not appear until the next stop.

### The LCD string hack does nothing

The LCD is written **once**, during `lcd_i2c_init` / `write_lcd_greeting`, before the loop starts. You must break at `0x1000027c` and redirect `r0` **before** that call runs. If you already let the target run into the loop, the LCD already shows the old strings. Restart the server parked at `main` (`BP_ADDR=0x10000234`) and reconnect, then set the breakpoint at `0x1000027c` while stopped.

Also confirm you wrote a NUL-terminated string. `lcd_puts` walks bytes until `*s == 0`; without the trailing `\x00`, it keeps sending RAM garbage to the PCF8574.

### The compiler did not keep `const` in `.rodata`

It is not supposed to in this build. `const int OTHER_FAV_NUM = 1337` becomes `movw r1, #1337` because the program never takes its address (`&OTHER_FAV_NUM`) and the value fits in a 16-bit immediate. A `const` only stays in `.rodata` when something forces it there — an address-taken `const`, an array, a pointer, or `volatile`. When you reverse a real binary, never assume a `const` is a memory load; check the instruction.

### The `movw` patch did not take

You patched the wrong byte. `movw` is a 32-bit instruction and its low immediate byte (`imm8`) is the **third** byte. The instruction starts at `0x10000296`, so the byte to change is `0x10000298` (`0x39 -> 0x40`). Patching `0x10000296` (the opcode) corrupts the instruction and the core will fault.

### The LCD shows garbage after patching

The replacement string is the wrong length or is missing its NUL. `"Reverse"` and `"Exploit"` are both seven characters, and the original eight-byte block is `52 65 76 65 72 73 65 00`. Write exactly `45 78 70 6c 6f 69 74 00`. A shorter string without padding runs into `"Engineering"` at `0x10003ef0`; a longer one overwrites it.

### The serial capture is garbage on macOS

Reading `/dev/cu.usbmodem*` with a bare `read()` returns garbage. Set **raw termios at 115200** first: clear canonical/echo flags, set `CLOCAL|CREAD`, and `B115200` on input and output. `screen /dev/cu.usbmodem* 115200` does all of this for you; a script must call `tcsetattr` itself. Once set, the capture reads clean `FAV_NUM: 42` lines.

### It worked for a second, then stopped (IDA's view desyncs)

This is the most common failure, and it has one main cause: **driving the core from the OpenOCD command port while IDA is connected.**

- If you send `reset run` from the port while attached, the core resets, runs, and halts at your breakpoint — but IDA never receives the stop event. Its sidebar keeps showing the *previous* location, so **Step** and **Resume** act on a stale PC and appear to do nothing.
- If the OpenOCD process dies (or you restart it) while attached, IDA keeps believing it is connected: the sidebar stays, but the menu shows **Pause** enabled and **Resume**/**Step** disabled because IDA last saw the target *running*.

Recovery: **Detach, then reconnect.** If Detach does nothing (the connection is already dead), restart IDA — its menu still shows a session that no longer exists.

Prevention:

- Stop at `main` with `BP_ADDR` on a fresh server start, not with `reset run` while attached.
- For loop addresses, set the breakpoint in the GUI and click **Resume**. Let IDA be the thing that starts the core.
- If you must reset, **Detach first**, `reset run`, then reconnect.
- Never leave a breakpoint on the PC you are about to step or resume from.

### The target "blows past" `main` and stops at `0x10003ab4` instead

`0x10003ab4` is inside `stdio_uart_out_flush`:

```asm
10003ab0: 4b02       ldr  r3, [pc, #8]  @ (10003abc <stdio_uart_out_flush+0xc>)
10003ab2: 681a       ldr  r2, [r3]
10003ab4: 6993       ldr  r3, [r2, #24] @ the core sits here while the UART drains
10003ab6: 071b       lsls  r3, r3, #28
10003ab8: d4fc       bmi.n 10003ab4 <stdio_uart_out_flush+0x4>
10003aba: 4770       bx lr
10003abc: 2000086c  .word 0x2000086c
```

That is the UART transmit-FIFO drain loop inside `printf`, so the core is running `main`'s loop and simply spends nearly all its time there. The breakpoint at `main` did not fire because `main`'s entry runs exactly **once per reset**. If you arm the breakpoint after the reset, or set it while the target is already running and just resume, the core is already past `main` and will never re-execute it. Either arm the breakpoint **before** resetting, or break inside the loop at `0x10000292`, which fires every iteration.

**`0x10003ab4` is not a function.** It is one instruction inside `stdio_uart_out_flush`, which starts at `0x10003ab0`. If IDA has created a function at `0x10003ab4` (for example because the debugger stopped at that PC), the decompiler shows garbage. Delete that bogus function (right-click it -> `Delete Function`, or put the cursor on it and press `U` to undefine) and reanalyze. The real function is `stdio_uart_out_flush` at `0x10003ab0`.

### The console floods with `Failed to read memory at 0xf0000000`

Core1 is exposed. The scripts must run with `USE_CORE=0`. Stop the server, confirm only `core0` is reported, restart, then restart IDA.

### `Connect to Remote Process` is greyed out and Pause does nothing

IDA is in a stale session, usually because the debug server restarted while attached. Quit and reopen IDA (or the `.i64`) and connect again.

### The decompiler still shows the old value after patching

Right-click the function and choose `Reanalyze`.

---

## Fallback: do the dynamic steps with GDB (macOS 27)

If IDA's debugger crashes on attach on macOS 27 (see Troubleshooting), you can still do the live hack with the ARM GDB from the toolchain, against the same OpenOCD server. The addresses and register values are identical to the GUI steps.

Start the debug server (Step 9), then in a new terminal:

```
arm-none-eabi-gdb
```

At the `(gdb)` prompt:

```
set architecture armv8-m.main
target extended-remote :3333
hbreak *0x10000292
continue
```

Do **not** run `monitor reset run` before `hbreak`. `0x10000292` is inside `main`'s loop, so the breakpoint fires on the next iteration with no reset. If you reset first, the core runs `main` and you will not catch it.

GDB stops at the `printf` call. Confirm the value, change it, and let it run:

```
info registers pc r1  # pc = 0x10000292, r1 = 0x2a
set $r1 = 0x2b
stepi
continue
```

The serial monitor prints `FAV_NUM: 43` for the iteration you changed — the same temporary live hack as editing `r1` in the IDA Registers widget. When you are done, press `Ctrl-C`, then `detach` and `quit`.

**If you specifically want to stop at `main` (`0x10000234`),** remember its entry runs only once per reset, so the breakpoint must be armed *before* the reset:

```
monitor reset halt
hbreak *0x10000234
continue
```

If you instead set it while the target is running and just `continue`, you will "blow past" `main` and catch the core inside `printf` — in this build at `0x10003ab4`, the `stdio_uart_out_flush` UART-drain loop.

`hbreak` sets a hardware breakpoint, which is required for read-only flash. It works from plain GDB because GDB sends the 2-byte length the Cortex-M33 comparators need. IDA's **GDB MI** adapter goes through the same GDB, so its GUI breakpoints work too; the old **GDB RSP** adapter was the one that sent a 1-byte length and could not set breakpoints here.

## Glossary

| Term | Definition |
| ---- | ---------- |
| **AAPCS** | ARM Architecture Procedure Call Standard — `r0`-`r3` for the first four arguments, `r0` for the return value |
| **`.bss`** | Section for uninitialized (or zero-initialized) static/global variables; zeroed by startup code |
| **`const`** | A source-level "read-only" qualifier; the compiler may still inline it as an immediate |
| **`.data`** | Section for initialized static/global variables; copied from flash to SRAM at boot |
| **`#define`** | Preprocessor text replacement performed before compilation; consumed by the compiler as a literal |
| **`.elf`** | Linked image with the symbol table; the ground truth for addresses and names |
| **`imm8`** | The low 8 bits of a `movw` immediate, stored in the third byte of the 32-bit instruction |
| **Immediate value** | A constant embedded directly in an instruction, not fetched from memory |
| **I2C** | Inter-Integrated Circuit — a two-wire (SDA/SCL) serial bus; open-drain, needs pull-ups |
| **Literal pool** | A block of 32-bit constants that Thumb-2 code reaches with PC-relative `ldr` |
| **`movs`** | 16-bit Thumb move that loads an 8-bit immediate (0-255) |
| **`movw`** | 32-bit Thumb-2 "move wide" that loads any 16-bit immediate (0-65535) |
| **Open-drain** | An output that can only pull a line LOW, not drive it HIGH; pull-ups restore HIGH |
| **PCF8574** | The I2C I/O expander on a typical 1602 LCD backpack; commonly at `0x27` |
| **`.rodata`** | Read-only section for constants and string literals; stays in flash |
| **SCL / SDA** | I2C Serial Clock and Serial Data lines |
| **Struct** | A user-defined type that groups related variables; the SDK uses one per I2C controller |
| **Thumb bit** | Bit 0 of a Cortex-M function pointer; selects Thumb instruction mode |
| **`typedef`** | Creates an alias for a type (for example `typedef struct i2c_inst i2c_inst_t`) |
| **UF2** | USB Flashing Format — the file format the Pico 2 bootloader accepts |
| **Vector table** | The first words of flash: initial stack pointer and exception vectors |

---

**Remember:** the ELF tells you what every address is, and the `.bin` is what you actually patch. Prove the behavior dynamically, read `r1` at the `printf` call, resolve the names from the ELF (including the `lcd_1602.c` symbols), then patch the bytes — `0x2a -> 0x2b`, `0x39 -> 0x40`, and the eight-byte LCD string — and flash.

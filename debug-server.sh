#!/usr/bin/env bash
#
# debug-server.sh - start OpenOCD as a live GDB server for the RP2350 via the Pico Debug Probe.
#
# Synopsis:
#   ./debug-server.sh
#
# Description:
#   Starts OpenOCD in the foreground as a long-running GDB server. It exposes
#   rp2350.dap.core0 on 127.0.0.1:3333 and leaves the core RUNNING, so that a
#   debugger (Binary Ninja, or plain GDB) can attach to a target that is already
#   executing and therefore has sane registers.
#
#   This script is deliberately NOT flash.sh. flash.sh programs flash and exits;
#   this one claims the probe and stays up so you can single-step, read memory,
#   and set breakpoints. Do not run both at once -- exactly one process may own
#   the debug probe.
#
#   The script ends with "reset run" rather than OpenOCD's default halt. This is
#   the single most important line in the file; see "Why reset run" below.
#
# Requirements:
#   - Pico Debug Probe (or any CMSIS-DAP / SWD adapter) connected to the target.
#   - OpenOCD with the rp2350 target script installed. See PICO_OPENOCD.
#   - Exactly one process may own the debug probe.
#   - A firmware image already programmed into flash (use flash.sh first).
#
# Environment variables:
#   PICO_OPENOCD   Directory containing the openocd binary AND its scripts/
#                  directory. Default: $HOME/.pico-sdk/openocd/0.12.0+dev
#                  macOS note: an OpenOCD on PATH is frequently the x86_64
#                  Homebrew build, which will not run under Rosetta on some
#                  setups and cannot talk to the ARM64 firmware tooling. The
#                  Pico SDK ships an arm64 build; point this at it explicitly.
#   ADAPTER_SPEED  SWD clock in kHz. Default: 24000.
#                  This is a read-mostly debug session, so a fast clock is fine
#                  and makes stepping noticeably smoother. Drop it (10000 or
#                  lower) if the link is flaky or you are using long dupont
#                  wires instead of the probe's own connector.
#   BP_ADDR        OPTIONAL address to halt at during the startup run, as 0x
#                  prefixed hex, e.g. 0x10000234 for main. Unset by default,
#                  which is the normal "attach to a running target" behavior.
#                  Arms a 2-byte hardware execute breakpoint just before the
#                  final "reset run", so the core runs from the vector table
#                  and stops there on its own, with no client attached yet.
#                  See "Why BP_ADDR is one-shot" below for the important
#                  limitation. Arming a length of 2 is mandatory: the Cortex-M33
#                  comparators are halfword-based and reject anything else.
#   USE_CORE       Which cores to expose to the client. Default: 0 (core0 only).
#                  Accepted values:
#                    0     core0 only            <- use this with Binary Ninja
#                    1     core1 only            <- rarely useful
#                    SMP   both cores as hwthreads <- see "Why USE_CORE=0"
#                  Do not change this to SMP when driving Binary Ninja. The
#                  explanation below is the whole reason this default is 0.
#
# Why USE_CORE=0 (core1 must be hidden from the client)
#   The RP2350 has two Cortex-M33 cores. Core1 does not start on its own: nothing
#   in a Pico SDK application releases it from reset unless the program
#   explicitly does so. If you expose it anyway (USE_CORE=SMP), its registers
#   read back as meaningless reset defaults:
#
#       core1:  pc 0x000000ec  sp 0xf0000000  xpsr 0x09000000  lr 0x00000147
#       core0:  pc 0x1000320c  sp 0x20081f38  xpsr 0xa9000000  lr 0x100030ab
#
#   Binary Ninja's register widget renders a memory preview for every register,
#   which means it treats each register value as an ADDRESS and reads it. The
#   core1 values above are not real addresses. Each read data-aborts, and
#   OpenOCD responds by tearing down and re-establishing the SWD debug port,
#   logging a pair of lines per fault:
#
#       Error: Failed to read memory at 0xf0000000
#       Info : SWD DPIDR 0x4c013477
#
#   That becomes a self-sustaining loop of roughly 450 fault-and-recover cycles
#   every two seconds, which makes the session unusable. It looks exactly like a
#   broken debugger or a broken firmware. It is neither: it is a configuration
#   mismatch, and hiding core1 removes it completely.
#
# Why reset run (the target must be released, not halted)
#   On RP2350, halting during reset stops the core at the boot ROM stub BEFORE
#   the stack pointer is loaded:
#
#       xPSR: 0xf9000000  pc: 0x00000088  msp: 0xf0000000  lr: 0xffffffff
#
#   Those values are garbage for every core, including core0. Attaching in that
#   window triggers the identical fault storm described above. Ending this
#   script with "reset run" means the core starts from the vector table and is
#   executing normally by the time you attach, so registers read back correct.
#
#   Consequence for the client: ATTACH WHILE THE TARGET IS RUNNING. Do not
#   press Reset/Restart in Binary Ninja -- it performs reset-halt and puts you
#   back in the garbage window. If you must reset, send "reset run" over the
#   OpenOCD telnet port (4444) instead:
#
#       nc 127.0.0.1 4444   then type:  reset run
#
#   ...or use debug-server.sh's restart instructions in WEEK04/WEEK04-BN.md.
#
# Why BP_ADDR is one-shot (read this before relying on it)
#   A breakpoint armed here DOES fire during the startup "reset run" and halts
#   the core at your address, so the server comes up parked there and your
#   debugger can simply attach and look at it. That part works.
#
#   What you do NOT get is a reusable breakpoint. As soon as any GDB client
#   connects, OpenOCD unconditionally flushes every breakpoint it is holding:
#
#       Info : accepting 'gdb' connection on tcp/3333
#       Debug: breakpoints.c:328 breakpoint_remove_all_internal():
#              [rp2350.dap.core0] Delete all breakpoints
#
#   So by the time Binary Ninja is up, the comparator is gone -- reading
#   0xE0002000 shows zeros, not your address. Consequences:
#
#     * The startup stop is single-use. You cannot Resume and re-catch the
#       same address.
#     * You cannot use BP_ADDR to stop in a loop that is already running,
#       because the core only passes that point once per reset.
#     * main (0x10000234) is a good BP_ADDR value precisely because it is
#       reached exactly once, right after reset.
#
#   To arm anything further, or to re-arm main, do it from the OpenOCD command
#   port AFTER your debugger has connected -- the order is the whole trick:
#
#       nc 127.0.0.1 4444
#       > bp 0x1000023e 2 hw
#       > reset run
#
#   Arming before the client connects does not work (flushed above), and in
#   plain GDB the equivalent "hbreak" is cleared by "detach" -- both were
#   verified by reading the FPB comparator registers back.
#
#   Related Binary Ninja bug: Binary Ninja's own breakpoints cannot be used on
#   this target at all. It sends Z0,<addr>,1 -- a 1-byte packet -- and OpenOCD
#   answers "only breakpoints of two bytes length supported". Both Toggle
#   Breakpoint (F2) and Add Hardware Breakpoint (F3) fail, at every address, and
#   the dialog's Size field is disabled so there is no UI way around it.
#   gdb_breakpoint_override hard does not change this. Hence the command-port
#   workflow described above.
#
# Why the remaining OpenOCD flags are set
#   gdb_breakpoint_override hard
#       Force every client breakpoint onto the Cortex-M33 hardware comparators
#       (the RP2350 has 8 breakpoints and 4 watchpoints). Without this, a client
#       may try to write a BKPT instruction into flash at 0x10000000, which is
#       read-only XIP memory, and the write fails.
#   gdb_memory_map disable
#       Stops the client probing the entire 32 MiB flash map on connect. Pure
#       Raspberry Pi guidance for suppressing spurious "Failed to read memory"
#       reports during target discovery.
#   cortex_m reset_config sysresetreq
#       The Cortex-M33 in the RP2350 has no VECTRESET. Without this, OpenOCD
#       warns on every reset:
#           VECTRESET is not supported on this Cortex-M core, using SYSRESETREQ
#           instead
#       NOTE: this MUST be the generic "cortex_m" command, not
#       "rp2350.dap.core1 cortex_m ...". With USE_CORE=0 the core1 target does
#       not exist, so a core1-scoped command aborts OpenOCD before "init" runs,
#       and the script exits silently having printed nothing useful.
#   adapter speed
#       See ADAPTER_SPEED above.
#
# Examples:
#   ./debug-server.sh
#   ADAPTER_SPEED=10000 ./debug-server.sh
#   PICO_OPENOCD=/opt/homebrew/bin ./debug-server.sh
#   BP_ADDR=0x10000234 ./debug-server.sh
#     Comes up halted at main (one-shot; see "Why BP_ADDR is one-shot").
#
# Exit status:
#   *  propagated from OpenOCD. A clean shutdown via Ctrl-C exits 0; an
#      OpenOCD configuration error exits non-zero.
#
# Related scripts:
#   flash.sh / flash.ps1   one-shot raw .bin programmer (exits when done)
#
# See also:
#   WEEK04/WEEK04-BN.md  the full walkthrough this script belongs to

set -euo pipefail

# --- configuration ---------------------------------------------------------

OCD="${PICO_OPENOCD:-$HOME/.pico-sdk/openocd/0.12.0+dev}"
SPEED="${ADAPTER_SPEED:-24000}"
USE_CORE="${USE_CORE:-0}"
BP_ADDR="${BP_ADDR:-}"

if [ -n "$BP_ADDR" ] && ! printf '%s' "$BP_ADDR" | grep -qiE '^0x[0-9a-f]+$'; then
  echo "error: BP_ADDR must be 0x-prefixed hex, e.g. 0x10000234 (got '$BP_ADDR')" >&2
  exit 1
fi

if [ ! -x "$OCD/openocd" ]; then
  echo "error: OpenOCD not found at $OCD/openocd" >&2
  echo "       set PICO_OPENOCD=/path/to/openocd (the directory containing the openocd binary)" >&2
  exit 1
fi

# --- announce, so the operator can verify intent before the target is touched --

echo "Starting OpenOCD GDB server on 127.0.0.1:3333 using $OCD/openocd"
echo "SWD adapter speed: ${SPEED} kHz"
echo "Cores exposed to GDB (USE_CORE): ${USE_CORE}"
echo "Target will be reset and released (reset run) - attach while it is running."

if [ -n "$BP_ADDR" ]; then
  echo "Startup breakpoint at ${BP_ADDR} (2-byte hardware execute, one-shot)."
  echo "  It fires during this startup reset run. Any client connecting later"
  echo "  causes OpenOCD to delete it, so arm further breakpoints on port 4444"
  echo "  after attaching:  bp <addr> 2 hw"
fi

# --- start -----------------------------------------------------------------
#
# Order matters: USE_CORE must be set BEFORE -f target/rp2350.cfg is read,
# because the target script branches on it when creating the DAP targets.
#
# "reset run" is last so it happens after init and after the target is
# examined, releasing the core rather than halting it. See the header for why.
#
# Any BP_ADDR breakpoint is inserted after "init" (the target must exist before
# a comparator can be programmed) but before "reset run" (so the core is already
# armed when it starts). The length must be 2: Cortex-M33 comparators reject
# other widths.

ocd_args=(
  -s "$OCD/scripts"
  -f interface/cmsis-dap.cfg
  -c "set USE_CORE ${USE_CORE}"
  -f target/rp2350.cfg
  # Drop the hwthread RTOS the RP2350 target script attaches to core0.
  #
  # target/rp2350.cfg creates core0 with "-rtos hwthread", which registers a
  # fake RTOS whose "current thread" is coreid+1 = 1. Binary Ninja single-steps
  # with the GDB packet "vCont;s" and no thread id, i.e. thread 0. OpenOCD's
  # gdb_server sees rtos->current_thread (1) != thread_id (0) and takes its
  # "fake step" path, replying with a stop without ever stepping the core:
  #
  #   gdb_server.c gdb_handle_vcont_packet(): fake step thread 0
  #
  # The result is that Step Into / Step Over in Binary Ninja does nothing: the
  # PC never moves. Clearing the RTOS removes the mismatch so the step is real.
  # Harmless for single-core use, which is all this lab does (USE_CORE=0).
  -c "rp2350.dap.core0 configure -rtos none"
  -c "adapter speed ${SPEED}"
  -c "gdb_memory_map disable"
  -c "gdb_breakpoint_override hard"
  -c "cortex_m reset_config sysresetreq"
  -c "init"
)

if [ -n "$BP_ADDR" ]; then
  ocd_args+=(-c "bp ${BP_ADDR} 2 hw")
fi

ocd_args+=(-c "reset run")

exec "$OCD/openocd" "${ocd_args[@]}"

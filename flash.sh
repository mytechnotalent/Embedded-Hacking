#!/usr/bin/env bash
#
# flash.sh - program a raw .bin into RP2350 XIP flash via the Pico Debug Probe + OpenOCD.
#
# Synopsis:
#   ./flash.sh <path-to-file.bin>
#
# Description:
#   Writes a headerless raw binary to the RP2350's external XIP flash starting at
#   physical address 0x10000000, then verifies the written bytes by reading them
#   back, then releases the core so the freshly programmed firmware runs.
#
#   A raw .bin carries no address information, no entry point and no section
#   table, so the load address MUST be supplied out of band. On the RP2350 the
#   only correct value is 0x10000000: that is where the boot ROM jumps after
#   pulling the reset vector out of the on-chip XIP window. Flashing anywhere
#   else produces a board that enumerates over USB and then does nothing.
#
#   The target must already be running the stock RP2350 bootrom (the default
#   state after power-up or after any BOOTSEL+UF2 reflash). OpenOCD attaches to
#   the bootrom via SWD, halts it, programs flash, verifies, and resets.
#
# Requirements:
#   - Pico Debug Probe (or any CMSIS-DAP / SWD adapter) connected to the target.
#   - OpenOCD with the rp2350 target script installed. The Pico SDK ships one;
#     see PICO_OPENOCD below.
#   - Exactly one process may own the debug probe. Close any other OpenOCD,
#     GDB, or IDE debug session first.
#
# Environment variables:
#   PICO_OPENOCD   Directory containing the openocd binary AND its scripts/
#                  directory. Default: $HOME/.pico-sdk/openocd/0.12.0+dev
#                  The path is used for BOTH the executable and -s scripts.
#   ADAPTER_SPEED  SWD clock in kHz. Default: 5000.
#                  5000 kHz is deliberately conservative. This is a write path,
#                  not a read-only debug session, and a marginal USB cable or
#                  long dupont run will produce spurious verify failures at
#                  higher clocks. Raise it (24000) for read-only work; if
#                  "Error: target not halted" or verify mismatches appear,
#                  lower it to 1000.
#
# Examples:
#   ./flash.sh 0x0005_intro-to-variables/build/0x0005_intro-to-variables.bin
#   ADAPTER_SPEED=1000 ./flash.sh build/hacked.bin
#   PICO_OPENOCD=/opt/homebrew/bin ./flash.sh build/hacked.bin
#
# Exit status:
#   0  flash written, verified, and core released
#   1  bad usage, missing input file, or OpenOCD not found
#   *  any other status is propagated from OpenOCD, so a failed verify or a
#      write error is visible to the caller rather than being swallowed.
#
# Related scripts:
#   debug-server.sh / debug-server.ps1  long-running GDB server for live
#                                       debugging with Binary Ninja
#
# See also:
#   WEEK04/WEEK04-BN.md  the full walkthrough this script belongs to

set -euo pipefail

# --- argument validation ---------------------------------------------------

BIN="${1:-}"
if [ -z "$BIN" ]; then
  echo "usage: $0 <path-to-file.bin>" >&2
  exit 1
fi

if [ ! -f "$BIN" ]; then
  echo "error: file not found: $BIN" >&2
  echo "       build it first:  cmake -B build -G Ninja -DPICO_BOARD=pico2 -DPICO_PLATFORM=rp2350 && cmake --build build" >&2
  exit 1
fi

# --- toolchain resolution --------------------------------------------------

OCD="${PICO_OPENOCD:-$HOME/.pico-sdk/openocd/0.12.0+dev}"
if [ ! -x "$OCD/openocd" ]; then
  echo "error: OpenOCD not found at $OCD/openocd" >&2
  echo "       set PICO_OPENOCD=/path/to/openocd (the directory containing the openocd binary)" >&2
  exit 1
fi

SPEED="${ADAPTER_SPEED:-5000}"

# --- program ---------------------------------------------------------------

# OpenOCD flag notes:
#   -f interface/cmsis-dap.cfg  the Pico Debug Probe is a CMSIS-DAP v1 device
#   -f target/rp2350.cfg        RP2350 dual Cortex-M33 + RP2350B0-style DAP;
#                               sets USE_CORE=SMP by default, which is fine here
#                               because we never hand uninitialised core1
#                               registers to a debugger. For live debugging use
#                               debug-server.sh, which forces USE_CORE=0.
#   -c "program BIN 0x10000000 verify reset exit"
#         program   the write
#         0x10000000  base address (see header)
#         verify    read back and compare every byte; a mismatch aborts
#         reset     reset the core so the new image starts at its vectors
#         exit      release the probe and return to the shell
#
# The adapter speed is deliberately lower here than in debug-server.sh; see
# ADAPTER_SPEED in the header.
echo "Flashing $BIN -> 0x10000000 using $OCD/openocd (SWD ${SPEED} kHz)"
"$OCD/openocd" \
  -s "$OCD/scripts" \
  -f interface/cmsis-dap.cfg \
  -f target/rp2350.cfg \
  -c "adapter speed ${SPEED}" \
  -c "program $BIN 0x10000000 verify reset exit"

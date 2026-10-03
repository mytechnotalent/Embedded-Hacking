<#
.SYNOPSIS
    Program a raw .bin into RP2350 XIP flash via the Pico Debug Probe and OpenOCD.

.DESCRIPTION
    Writes a headerless raw binary to the RP2350's external XIP flash starting at
    physical address 0x10000000, verifies the written bytes by reading them back,
    then releases the core so the freshly programmed firmware runs.

    A raw .bin carries no address information, no entry point and no section
    table, so the load address MUST be supplied out of band. On the RP2350 the
    only correct value is 0x10000000: that is where the boot ROM jumps after
    pulling the reset vector out of the on-chip XIP window. Flashing anywhere
    else produces a board that enumerates over USB and then does nothing.

    The target must already be running the stock RP2350 bootrom (the default
    state after power-up, or after any BOOTSEL + UF2 reflash). OpenOCD attaches
    to the bootrom over SWD, halts it, programs flash, verifies, and resets.

    This is the macOS / Linux equivalent of flash.sh. The two must stay in
    lockstep: same base address, same verify, same conservative SWD clock.

.PARAMETER Bin
    Mandatory. Path to the raw .bin image to program.

.ENVIRONMENT
    PICO_OPENOCD   Directory containing openocd.exe AND its scripts\ directory.
                   Default: $env:USERPROFILE\.pico-sdk\openocd\0.12.0+dev
                   The path is used for BOTH the executable and the -s scripts
                   argument.
    ADAPTER_SPEED  SWD clock in kHz. Default: 5000.
                   5000 kHz is deliberately conservative. This is a write path,
                   not a read-only debug session, and a marginal USB cable or
                   long dupont run produces spurious verify failures at higher
                   clocks. Raise it (24000) for read-only work; if
                   "Error: target not halted" or verify mismatches appear,
                   lower it to 1000.

.EXAMPLE
    .\flash.ps1 -Bin 0x0005_intro-to-variables\build\0x0005_intro-to-variables.bin

.EXAMPLE
    $env:ADAPTER_SPEED=1000; .\flash.ps1 -Bin build\hacked.bin

.EXAMPLE
    $env:PICO_OPENOCD="C:\openocd\bin"; .\flash.ps1 -Bin build\hacked.bin

.NOTES
    Requirements:
      * Pico Debug Probe (or any CMSIS-DAP / SWD adapter) connected to the target.
      * OpenOCD with the rp2350 target script installed.
      * The Debug Probe must use the WinUSB driver. If OpenOCD reports
        "unable to open CMSIS-DAP device", install it with Zadig
        (https://zadig.akeo.ie/), selecting "Debug Probe (CMSIS-DAP)" -> WinUSB.
      * Exactly one process may own the debug probe. Close any other OpenOCD,
        GDB, or IDE debug session first.

    Exit status:
      0    flash written, verified, and core released
      1    input file missing or OpenOCD not found
      *    any other status is propagated from OpenOCD, so a failed verify or a
           write error is visible to the caller rather than being swallowed.

    Related scripts:
      debug-server.ps1  long-running GDB server for live debugging with
                        Binary Ninja. Use that instead of this script when you
                        need to single-step.

    See also:
      WEEK04\WEEK04-BN.md  the full walkthrough this script belongs to
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$Bin
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# --- argument validation ---------------------------------------------------

if (-not (Test-Path -PathType Leaf $Bin)) {
    Write-Error "file not found: $Bin"
    Write-Host "build it first: cmake -B build -G Ninja -DPICO_BOARD=pico2 -DPICO_PLATFORM=rp2350 ; cmake --build build"
    exit 1
}

# --- toolchain resolution --------------------------------------------------

$OCD = if ($env:PICO_OPENOCD) { $env:PICO_OPENOCD } else { "$env:USERPROFILE\.pico-sdk\openocd\0.12.0+dev" }
if (-not (Test-Path "$OCD\openocd.exe")) {
    Write-Error "OpenOCD not found at $OCD\openocd.exe (set PICO_OPENOCD to the directory containing openocd.exe)"
    exit 1
}

$SPEED = if ($env:ADAPTER_SPEED) { $env:ADAPTER_SPEED } else { "5000" }

# --- program ---------------------------------------------------------------

# OpenOCD flag notes:
#   -f interface/cmsis-dap.cfg  the Pico Debug Probe is a CMSIS-DAP v1 device
#   -f target/rp2350.cfg        RP2350 dual Cortex-M33 + RP2350B0-style DAP;
#                               sets USE_CORE=SMP by default, which is fine here
#                               because we never hand uninitialised core1
#                               registers to a debugger. For live debugging use
#                               debug-server.ps1, which forces USE_CORE=0.
#   -c "program BIN 0x10000000 verify reset exit"
#         program       the write
#         0x10000000    base address (see .DESCRIPTION)
#         verify        read back and compare every byte; a mismatch aborts
#         reset         reset the core so the new image starts at its vectors
#         exit          release the probe and return to the shell
#
# The adapter speed is deliberately lower here than in debug-server.ps1; see
# ADAPTER_SPEED in .ENVIRONMENT.
Write-Host "Flashing $Bin -> 0x10000000 using $OCD\openocd.exe (SWD $SPEED kHz)"

& "$OCD\openocd.exe" `
  -s "$OCD\scripts" `
  -f interface/cmsis-dap.cfg `
  -f target/rp2350.cfg `
  -c "adapter speed $SPEED" `
  -c "program $Bin 0x10000000 verify reset exit"

exit $LASTEXITCODE

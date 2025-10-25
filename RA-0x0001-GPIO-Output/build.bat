@echo off
REM ==============================================================================
REM FILE: build.bat
REM
REM DESCRIPTION:
REM Build script for RP2350.
REM
REM BRIEF:
REM Automates the process of assembling, linking, and generating UF2 firmware.
REM
REM AUTHOR: Kevin Thomas
REM CREATION DATE: October 5, 2025
REM UPDATE DATE: October 5, 2025
REM ==============================================================================

echo Building GPIO16 blink...

REM ==============================================================================
REM Assemble Source Files
REM ==============================================================================
arm-none-eabi-as -mcpu=cortex-m33 -mthumb main.s -o gpio16_blink.o
if errorlevel 1 goto error

arm-none-eabi-as -mcpu=cortex-m33 -mthumb image_def.s -o image_def.o
if errorlevel 1 goto error

REM ==============================================================================
REM Link Object Files
REM ==============================================================================
arm-none-eabi-ld -T linker.ld gpio16_blink.o image_def.o -o gpio16_blink.elf
if errorlevel 1 goto error

REM ==============================================================================
REM Create Raw Binary from ELF
REM ==============================================================================
arm-none-eabi-objcopy -O binary gpio16_blink.elf gpio16_blink.bin
if errorlevel 1 goto error

REM ==============================================================================
REM Create UF2 Image for RP2350
REM -b 0x10000000 : base address
REM -f 0xe48bff59 : RP2350 family ID
REM ==============================================================================
python uf2conv.py -b 0x10000000 -f 0xe48bff59 -o gpio16_blink.uf2 gpio16_blink.bin
if errorlevel 1 goto error

REM ==============================================================================
REM Success Message and Flashing Instructions
REM ==============================================================================
echo.
echo =================================
echo SUCCESS! Created gpio16_blink.uf2
echo =================================
echo.
echo To flash via UF2:
echo   1. Hold BOOTSEL button
echo   2. Plug in USB
echo   3. Copy gpio16_blink.uf2 to RP2350 drive
echo.
echo To flash via OpenOCD (debug probe):
echo   openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000" -c "program gpio16_blink.elf verify reset exit"
echo.
goto end

REM ==============================================================================
REM Error Handling
REM ==============================================================================
:error
echo.
echo BUILD FAILED!
echo.

:end

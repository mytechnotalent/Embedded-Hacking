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

echo Building...

REM ==============================================================================
REM Assemble Source Files
REM ==============================================================================
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb vector_table.s -o vector_table.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb reset_handler.s -o reset_handler.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb stack.s -o stack.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb xosc.s -o xosc.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb reset.s -o reset.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb coprocessor.s -o coprocessor.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb uart.s -o uart_module.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb gpio.s -o gpio.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb delay.s -o delay.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb main.s -o main.o
if errorlevel 1 goto error
arm-none-eabi-as -g -mcpu=cortex-m33 -mthumb image_def.s -o image_def.o
if errorlevel 1 goto error

REM ==============================================================================
REM Link Object Files
REM ==============================================================================
arm-none-eabi-ld -g -T linker.ld vector_table.o reset_handler.o stack.o xosc.o reset.o coprocessor.o uart_module.o gpio.o delay.o main.o image_def.o -o uart.elf
if errorlevel 1 goto error

REM ==============================================================================
REM Create Raw Binary from ELF
REM ==============================================================================
arm-none-eabi-objcopy -O binary uart.elf uart.bin
if errorlevel 1 goto error

REM ==============================================================================
REM Create UF2 Image for RP2350
REM -b 0x10000000 : base address
REM -f 0xe48bff59 : RP2350 family ID
REM ==============================================================================
python uf2conv.py -b 0x10000000 -f 0xe48bff59 -o uart.uf2 uart.bin
if errorlevel 1 goto error

REM ==============================================================================
REM Success Message and Flashing Instructions
REM ==============================================================================
echo.
echo =================================
echo SUCCESS! Created uart.uf2
echo =================================
echo.
echo To flash via UF2:
echo   1. Hold BOOTSEL button
echo   2. Plug in USB
echo   3. Copy uart.uf2 to RP2350 drive
echo.
echo To flash via OpenOCD (debug probe):
echo   openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000" -c "program uart.elf verify reset exit"
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

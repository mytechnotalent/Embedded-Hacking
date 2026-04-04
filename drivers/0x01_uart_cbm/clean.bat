@echo off
REM ==============================================================================
REM FILE: clean.bat
REM
REM DESCRIPTION:
REM Clean script for RP2350 bare-metal C UART driver.
REM
REM BRIEF:
REM Removes all build artifacts and generated files.
REM
REM AUTHOR: Kevin Thomas
REM CREATION DATE: 2025
REM UPDATE DATE: 2025
REM ==============================================================================

echo Cleaning build artifacts...

REM Remove object files
if exist *.o del /Q *.o

REM Remove ELF file
if exist uart.elf del /Q uart.elf

REM Remove binary file
if exist uart.bin del /Q uart.bin

REM Remove UF2 file
if exist uart.uf2 del /Q uart.uf2

echo Clean complete!

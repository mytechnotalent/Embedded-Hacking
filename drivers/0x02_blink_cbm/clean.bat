@echo off
REM ==============================================================================
REM FILE: clean.bat
REM
REM DESCRIPTION:
REM Clean script for RP2350 bare-metal C Blink driver.
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
if exist blink.elf del /Q blink.elf

REM Remove binary file
if exist blink.bin del /Q blink.bin

REM Remove UF2 file
if exist blink.uf2 del /Q blink.uf2

echo Clean complete!

@echo off
REM ==============================================================================
REM FILE: clean.bat
REM
REM DESCRIPTION:
REM Clean script for RP2350.
REM
REM BRIEF:
REM Removes all build artifacts including object files, ELF, binary, and UF2.
REM
REM AUTHOR: Kevin Thomas
REM CREATION DATE: November 27, 2025
REM UPDATE DATE: November 27, 2025
REM ==============================================================================

echo Cleaning build artifacts...

REM ==============================================================================
REM Delete Object Files
REM ==============================================================================
if exist *.o (
  del /Q *.o
  echo Deleted object files
) else (
  echo No object files found
)

REM ==============================================================================
REM Delete ELF Files
REM ==============================================================================
if exist *.elf (
  del /Q *.elf
  echo Deleted ELF files
) else (
  echo No ELF files found
)

REM ==============================================================================
REM Delete Binary Files
REM ==============================================================================
if exist *.bin (
  del /Q *.bin
  echo Deleted binary files
) else (
  echo No binary files found
)

REM ==============================================================================
REM Delete UF2 Files
REM ==============================================================================
if exist *.uf2 (
  del /Q *.uf2
  echo Deleted UF2 files
) else (
  echo No UF2 files found
)

echo.
echo Clean complete!

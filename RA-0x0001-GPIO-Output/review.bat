@echo off
REM ============================================================================
REM FILE: review.bat
REM
REM DESCRIPTION:
REM Review script for RP2350 ELF and BIN outputs.
REM Runs a suite of GNU binutils tools to inspect ELF and BIN files.
REM Dumps results into text files for easy review.
REM
REM AUTHOR: Kevin Thomas
REM CREATION DATE: October 5, 2025
REM UPDATE DATE: October 5, 2025
REM ============================================================================

set ELF_FILE=gpio16_blink.elf
set BIN_FILE=gpio16_blink.bin
set OUT_DIR=review_reports

if not exist %OUT_DIR% mkdir %OUT_DIR%

echo Reviewing %ELF_FILE% and %BIN_FILE% ...

REM ============================================================================
REM ELF inspection tools
REM ============================================================================
arm-none-eabi-readelf -a %ELF_FILE%   > %OUT_DIR%\readelf_all.txt
arm-none-eabi-readelf -h %ELF_FILE%   > %OUT_DIR%\readelf_header.txt
arm-none-eabi-readelf -S %ELF_FILE%   > %OUT_DIR%\readelf_sections.txt
arm-none-eabi-readelf -s %ELF_FILE%   > %OUT_DIR%\readelf_symbols.txt
arm-none-eabi-readelf -r %ELF_FILE%   > %OUT_DIR%\readelf_relocs.txt

arm-none-eabi-objdump -x %ELF_FILE%   > %OUT_DIR%\objdump_headers.txt
arm-none-eabi-objdump -d %ELF_FILE%   > %OUT_DIR%\objdump_disasm.txt
arm-none-eabi-objdump -s %ELF_FILE%   > %OUT_DIR%\objdump_fullhex.txt
arm-none-eabi-objdump -h %ELF_FILE%   > %OUT_DIR%\objdump_sections.txt

arm-none-eabi-nm -n %ELF_FILE%        > %OUT_DIR%\nm_symbols.txt
arm-none-eabi-size %ELF_FILE%         > %OUT_DIR%\size.txt
arm-none-eabi-strings %ELF_FILE%      > %OUT_DIR%\strings.txt

REM ============================================================================
REM BIN inspection tools
REM ============================================================================
xxd -g4 -l 256 %BIN_FILE%             > %OUT_DIR%\bin_hexdump.txt

REM ============================================================================
REM Summary
REM ============================================================================
echo.
echo ========================================
echo REVIEW COMPLETE
echo Reports written to %OUT_DIR%\
echo ========================================
echo.
echo Key files:
echo   - readelf_all.txt       (everything about ELF)
echo   - objdump_disasm.txt    (disassembly)
echo   - nm_symbols.txt        (symbols)
echo   - size.txt              (section sizes)
echo   - bin_hexdump.txt       (first 256 bytes of BIN)
echo.
goto end

:error
echo.
echo REVIEW FAILED!
echo.

:end

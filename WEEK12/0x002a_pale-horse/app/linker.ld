/**
 * FILE: linker.ld
 *
 * DESCRIPTION:
 * PALE HORSE Stage-3 Application Linker Script.
 *
 * BRIEF:
 * Places the application vector table at 0x10001000 (the loader jumps here),
 * then code and read-only data. Uninitialized data lives in SRAM.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

ENTRY(App_Reset)

__APP_BASE = 0x10001000;

MEMORY
{
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 512K
  BSS   (rw)  : ORIGIN = 0x20000200, LENGTH = 64K
  FLASH (rx)  : ORIGIN = 0x10001000, LENGTH = 0x3e000
}

SECTIONS
{
  . = ORIGIN(FLASH);

  .vectors :
  {
    KEEP(*(.vectors))
  } > FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text*)
    *(.rodata*)
    *(.ARM.attributes)
  } > FLASH

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    *(.bss*)
    *(COMMON)
  } > BSS

  ASSERT((ADDR(.bss) >= 0x20000040), ".bss overlaps the relocated vector table")
}

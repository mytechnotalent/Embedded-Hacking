/**
 * FILE: linker.ld
 *
 * DESCRIPTION:
 * PALE HORSE Stage-2 Linker Script.
 *
 * BRIEF:
 * Places the IMAGE_DEF block at flash base 0x10000000 and the vector table
 * at 0x10000100, then the loader code, and fails the link if the loader
 * would run into the stage-3 application at 0x10001000.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

ENTRY(Stage2_Reset)

__XIP_BASE   = 0x10000000;
__SRAM_BASE  = 0x20000000;
__SRAM_SIZE  = 512K;

MEMORY
{
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 512K
  FLASH (rx)  : ORIGIN = 0x10000000, LENGTH = 0x1000
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

  .embedded_block :
  {
    KEEP(*(.embedded_block))
  } > FLASH

  ASSERT((ADDR(.text) + SIZEOF(.text)) <= 0x10001000,
         "Stage-2 loader overlaps the stage-3 application at 0x10001000")

  .stack (NOLOAD) : { . = ALIGN(8); } > RAM
}

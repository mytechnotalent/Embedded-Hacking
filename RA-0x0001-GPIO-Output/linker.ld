/**
 * FILE: linker.ld
 *
 * DESCRIPTION:
 * RP2350 Minimal Linker Script for bare‑metal development.
 * Ensures the boot ROM accepts and runs the image by:
 *   - Placing the IMAGE_DEF block first at 0x10000000
 *   - Aligning the vector table to a 128‑byte boundary within the first 4 KB
 *   - Defining a non‑secure stack region in SRAM
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: October 5, 2025
 * UPDATE DATE: October 5, 2025
 *
 * KEY FEATURES:
 *   - FLASH origin at 0x10000000 (XIP base), 32 MB length
 *   - RAM origin at 0x20000000, 512 KB non‑secure window
 *   - IMAGE_DEF block emitted before all other sections
 *   - Vector table alignment and assertion to satisfy ROM requirements
 *   - Symbols for __StackTop, __StackLimit, and __Vectors provided
 *
 * REFERENCE:
 *   RP2350 Datasheet, Section 5.9.5 Minimum viable image metadata
 *
 * NOTES:
 *   This script is intentionally minimal to support reproducible,
 *   SDK‑free bare‑metal builds. It pairs with a standalone IMAGE_DEF
 *   assembly file and a vector table defined in your startup code.
 */

ENTRY(Reset_Handler)

/**
 * Define memory regions.
 */
__XIP_BASE   = 0x10000000;
__XIP_SIZE   = 32M;

__SRAM_BASE  = 0x20000000;
__SRAM_SIZE  = 512K;                        /* non-secure window */
__STACK_SIZE = 32K;

MEMORY
{
  RAM   (rwx) : ORIGIN = __SRAM_BASE, LENGTH = __SRAM_SIZE
  FLASH (rx)  : ORIGIN = __XIP_BASE,  LENGTH = __XIP_SIZE
}

/**
 * Program headers.
 */
PHDRS
{
  text PT_LOAD FLAGS(5);                    /* RX */
}

/**
 * Section placement.
 */
SECTIONS
{
  . = ORIGIN(FLASH);

  /**
   * Minimal IMAGE_DEF must be first.
   */
  .embedded_block :
  {
    KEEP(*(.embedded_block))
  } > FLASH :text

  /**
   * Force the vector table section start to a 128-byte boundary.
   */
  .vectors ALIGN(128) :
  {
    KEEP(*(.vectors))
  } > FLASH :text

  ASSERT(((ADDR(.vectors) - ORIGIN(FLASH)) < 0x1000),
         "Vector table must be in first 4KB of flash")

  /**
   * Text and read-only data.
   */
  .text :
  {
    . = ALIGN(4);
    *(.text*)
    *(.rodata*)
    KEEP(*(.ARM.attributes))
  } > FLASH :text

  /**
   * Non-secure stack symbols.
   */
  __StackTop   = ORIGIN(RAM) + LENGTH(RAM); /* 0x20080000 */
  __StackLimit = __StackTop - __STACK_SIZE;
  __stack      = __StackTop;

  .stack (NOLOAD) : { . = ALIGN(8); } > RAM

  PROVIDE(__Vectors = ADDR(.vectors));
}

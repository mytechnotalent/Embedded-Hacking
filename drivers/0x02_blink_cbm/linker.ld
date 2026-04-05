/**
  ******************************************************************************
  * @file    linker.ld
  * @author  Kevin Thomas
  * @brief   Minimal linker script for bare-metal RP2350 development.
  *
  *          Defines FLASH (XIP 32 MB) and RAM (520 kB SRAM) regions.
  *          The vector table is placed at the start of flash (0x10000000).
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/**
  * Entry point.
  */
ENTRY(Reset_Handler)

/**
  * Define memory regions.
  */
__XIP_BASE   = 0x10000000;
__XIP_SIZE   = 32M;

__SRAM_BASE  = 0x20000000;
__SRAM_SIZE  = 520K;
__STACK_SIZE = 32K;

/**
  * Memory layout.
  */
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
  text PT_LOAD FLAGS(5);
}

/**
  * Section placement.
  */
SECTIONS
{
  . = ORIGIN(FLASH);

  /**
    * Vector table MUST be first at 0x10000000.
    */
  .vectors :
  {
    KEEP(*(.vectors))
  } > FLASH :text

  /**
    * Verify vector table placement.
    */
  ASSERT((ADDR(.vectors) == ORIGIN(FLASH)),
         "Vector table must be at start of flash (0x10000000)")

  /**
    * Text and read-only data.
    */
  .text :
  {
    . = ALIGN(4);
    *(.text*)
    *(.rodata*)
    KEEP(*(.init))
    KEEP(*(.fini))
    KEEP(*(.ARM.attributes))
  } > FLASH :text

  /**
    * IMAGE_DEF block at end of code.
    */
  .embedded_block :
  {
    KEEP(*(.embedded_block))
  } > FLASH :text

  /**
    * Non-secure stack symbols.
    */
  __StackTop   = ORIGIN(RAM) + LENGTH(RAM);
  __StackLimit = __StackTop - __STACK_SIZE;
  __stack      = __StackTop;
  _stack_top   = __StackTop;

  /**
    * Stack section (no load).
    */
  .stack (NOLOAD) : { . = ALIGN(8); } > RAM

  /**
    * Provide vector table symbol to startup code.
    */
  PROVIDE(__Vectors = ADDR(.vectors));
}

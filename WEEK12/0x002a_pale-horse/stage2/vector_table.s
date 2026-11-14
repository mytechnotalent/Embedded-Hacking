/**
 * FILE: vector_table.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-2 Vector Table.
 *
 * BRIEF:
 * The initial stack pointer and the stage-2 reset handler. Placed at
 * 0x10000100 by the stage-2 linker script.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

/**
 * Initialize the .vectors section.
 */
.section .vectors, "ax"                          // vector table section
.align 2                                         // align to 4-byte boundary

/**
 * Vector table.
 */
.global _vectors                                 // export _vectors symbol
_vectors:
  .word STACK_TOP                                // initial stack pointer
  .word Stage2_Reset + 1                         // reset handler (Thumb bit set)

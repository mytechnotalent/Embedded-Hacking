/**
 * FILE: vector_table.s
 *
 * DESCRIPTION:
 * RP2350 Vector Table.
 * 
 * BRIEF:
 * Defines the vector table for the RP2350 containing the initial
 * stack pointer and reset handler entry point.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 27, 2025
 * UPDATE DATE: November 27, 2025
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

/**
 * Initialize the .vectors section. The .vectors section contains vector
 * table and Reset_Handler.
 */
.section .vectors, "ax"                          // vector table section
.align 2                                         // align to 4-byte boundary

/**
 * Vector table section.
 */
.global _vectors                                 // export _vectors symbol
_vectors:
  .word STACK_TOP                                // initial stack pointer
  .word Reset_Handler + 1                        // reset handler (Thumb bit set)

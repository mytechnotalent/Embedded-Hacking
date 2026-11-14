/**
 * FILE: vector_table.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-3 Vector Table.
 *
 * BRIEF:
 * Initial stack pointer and the application reset handler. Placed at
 * 0x10001000 by the stage-3 linker script. The table is copied into SRAM
 * by the reset handler and installed via VTOR.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

.section .vectors, "ax"                          // vector table section
.align 2                                         // align to 4-byte boundary

.global _app_vectors                             // export the app vector table
_app_vectors:
  .word STACK_TOP                                // initial stack pointer
  .word App_Reset + 1                            // reset handler (Thumb bit set)

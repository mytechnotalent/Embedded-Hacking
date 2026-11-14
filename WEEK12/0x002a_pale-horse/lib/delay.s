/**
 * FILE: delay.s
 *
 * DESCRIPTION:
 * RP2350 Delay Functions.
 * 
 * BRIEF:
 * Provides millisecond delay functions based on a 14.5MHz clock.
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
 * Initialize the .text section. 
 * The .text section contains executable code.
 */
.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Delay_MS.
 *
 * @details Delays for r0 milliseconds. Conversion: loop_count = ms * 3600
 *          based on a 14.5MHz clock.
 *
 * @param   r0 - milliseconds
 * @retval  None
 */
.global Delay_MS
.type Delay_MS, %function
Delay_MS:
.Delay_MS_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.Delay_MS_Check:
  cmp   r0, #0                                   // if MS is not valid, return
  ble   .Delay_MS_Done                           // branch if less or equal to 0 
.Delay_MS_Setup:
  ldr   r4, =3600                                // loops per MS based on 14.5MHz clock
  mul   r5, r0, r4                               // MS * 3600
.Delay_MS_Loop:
  subs  r5, r5, #1                               // decrement counter
  bne   .Delay_MS_Loop                           // branch until zero
.Delay_MS_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

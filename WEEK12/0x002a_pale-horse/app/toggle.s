/**
 * FILE: toggle.s
 *
 * DESCRIPTION:
 * PALE HORSE GPIO toggle helper.
 *
 * BRIEF:
 * Configures a GPIO as an SIO output, then toggles it with the SIO output
 * XOR register. Placed last in the link so existing code addresses do not
 * move.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 14, 2026
 * UPDATE DATE: November 14, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

/**
 * Initialize the .toggle section.
 * The .toggle section holds the GPIO toggle helper.
 */
.section .toggle,"ax",%progbits                  // code placed after .rodata
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Configure and toggle a GPIO.
 *
 * @param   r0 - GPIO number
 * @retval  None
 */
.global GPIO_Toggle
.type GPIO_Toggle, %function
GPIO_Toggle:
.GPIO_Toggle_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr
  mov   r4, r0                                   // save GPIO number
  lsl   r0, r4, #2                               // pad offset = 4 + pin*4
  add   r0, r0, #4
  lsl   r1, r4, #3                               // ctrl offset = 4 + pin*8
  add   r1, r1, #4
  mov   r2, r4                                   // GPIO number
  bl    GPIO_Config                              // pad, function select, OE
  ldr   r0, =SIO_BASE                            // SIO base
  mov   r1, #1                                   // bit mask
  lsl   r1, r4                                   // 1 << GPIO
  str   r1, [r0, #SIO_GPIO_OUT_XOR]              // toggle output
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr
  bx    lr                                       // return
.size GPIO_Toggle, . - GPIO_Toggle

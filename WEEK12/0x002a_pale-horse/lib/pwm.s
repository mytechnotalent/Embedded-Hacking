/**
 * FILE: pwm.s
 *
 * DESCRIPTION:
 * RP2350 PWM Functions (SG90 servo).
 *
 * BRIEF:
 * Configures GP6 as PWM slice 3 and maps a 0-180 degree angle to a 1-2 ms
 * pulse at ~50 Hz. Register offsets are taken from the RP2350 SVD.
 *
 * NOTE:
 * Clock divider, top and pulse counts assume CLK_PERI = 14.5 MHz (XOSC).
 * Validate the pulse width with a scope or the servo before trusting it.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Initialize the servo PWM output on GP6.
 *
 * @details Sets GP6 to the PWM function, enables the pad, and configures
 *          PWM slice 3 (divider, top, compare) and enables the slice.
 *
 * @param   None
 * @retval  None
 */
.global PWM_Servo_Init
.type PWM_Servo_Init, %function
PWM_Servo_Init:
.PWM_Servo_Init_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.PWM_Servo_Init_Pin:
  ldr   r0, =IO_BANK0_BASE                       // GPIO function select base
  ldr   r1, =4                                   // FUNCSEL 4 -> PWM
  str   r1, [r0, #0x34]                          // GPIO6_CTRL = PWM
  ldr   r0, =PADS_BANK0_BASE                     // pad control base
  ldr   r1, =0x40                                // IE = 1, drive default
  str   r1, [r0, #0x1c]                          // PADS GPIO6
.PWM_Servo_Init_Slice:
  ldr   r0, =PWM_BASE                            // PWM base
  ldr   r1, =5                                   // integer divider 5
  lsl   r1, r1, #16                              // divider into high half
  str   r1, [r0, #0x40]                          // CH3_DIV
  ldr   r1, =57999                               // top = ~20 ms at div 5
  str   r1, [r0, #0x4c]                          // CH3_TOP
  ldr   r1, =2900                                // default 1.0 ms pulse
  str   r1, [r0, #0x48]                          // CH3_CC
  ldr   r1, =1                                   // enable the slice
  str   r1, [r0, #0x3c]                          // CH3_CSR
  ldr   r0, =PWM_BASE                            // PWM base
  ldr   r1, [r0, #PWM_EN]                        // read enable register
  orr   r1, r1, #(1<<3)                          // enable slice 3
  str   r1, [r0, #PWM_EN]                        // write enable register
.PWM_Servo_Init_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Set the servo angle.
 *
 * @details Maps 0..180 degrees to a 1.0..2.0 ms pulse (2900..5800 counts).
 *
 * @param   r0 - angle in degrees (0..180)
 * @retval  None
 */
.global PWM_Set_Angle
.type PWM_Set_Angle, %function
PWM_Set_Angle:
.PWM_Set_Angle_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  cmp   r0, #180                                 // clamp above 180
  bls   .PWM_Set_Angle_Scale                     // within range -> scale
  mov   r0, #180                                 // limit to 180
.PWM_Set_Angle_Scale:
  ldr   r1, =2900                                // pulse span counts (1.0..2.0 ms)
  mul   r2, r0, r1                               // angle * span
  ldr   r3, =180                                 // degrees
  udiv  r2, r2, r3                               // scale to 0..2900
  ldr   r1, =2900                                // base pulse (1.0 ms)
  add   r2, r2, r1                               // base + scaled
  ldr   r0, =PWM_BASE                            // PWM base
  str   r2, [r0, #0x48]                          // CH3_CC
.PWM_Set_Angle_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

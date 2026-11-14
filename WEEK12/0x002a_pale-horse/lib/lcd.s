/**
 * FILE: lcd.s
 *
 * DESCRIPTION:
 * RP2350 1602 LCD (PCF8574 I2C backpack) Functions.
 *
 * BRIEF:
 * Drives a 1602 character LCD over I2C1 in 4-bit mode. The PCF8574 bit
 * mapping is: P0 = RS, P1 = RW, P2 = EN, P3 = backlight, P4-P7 = D4-D7.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

.equ LCD_RS,          0x01                       // register select
.equ LCD_RW,          0x02                       // read/write
.equ LCD_EN,          0x04                       // enable strobe
.equ LCD_BL,          0x08                       // backlight

.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Send one nibble to the LCD with an enable pulse.
 *
 * @param   r0 - nibble value (low 4 bits, already shifted) | RS | BL
 * @retval  None
 */
.global LCD_Nibble
.type LCD_Nibble, %function
LCD_Nibble:
.LCD_Nibble_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  mov   r4, r0                                   // save payload
  orr   r0, r4, #LCD_EN                          // EN high
  bl    I2C_Write                                // latch data
  bic   r0, r4, #LCD_EN                          // EN low
  bl    I2C_Write                                // complete pulse
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Send a byte to the LCD (high nibble then low nibble).
 *
 * @param   r0 - byte
 * @param   r1 - 0 = command, 1 = data
 * @retval  None
 */
.global LCD_Byte
.type LCD_Byte, %function
LCD_Byte:
.LCD_Byte_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  mov   r4, r0                                   // byte
  mov   r5, r1                                   // rs flag
  ldr   r6, =LCD_BL                              // backlight bit
  cmp   r5, #0                                   // data?
  beq   .LCD_Byte_High                           // command -> no RS
  orr   r6, r6, #LCD_RS                          // data -> set RS
.LCD_Byte_High:
  and   r0, r4, #0xf0                            // high nibble in D4-D7
  orr   r0, r0, r6                               // add RS/backlight
  bl    LCD_Nibble                               // send high nibble
  lsl   r0, r4, #4                               // low nibble into D4-D7
  and   r0, r0, #0xf0                            // mask
  orr   r0, r0, r6                               // add RS/backlight
  bl    LCD_Nibble                               // send low nibble
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Initialize the LCD in 4-bit mode.
 *
 * @param   None
 * @retval  None
 */
.global LCD_Init
.type LCD_Init, %function
LCD_Init:
.LCD_Init_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  bl    I2C_Init                                 // bring up I2C1
  ldr   r0, =0x30                                // reset sequence step 1
  bl    LCD_Nibble                               // send
  ldr   r0, =0x30                                // reset sequence step 2
  bl    LCD_Nibble                               // send
  ldr   r0, =0x30                                // reset sequence step 3
  bl    LCD_Nibble                               // send
  ldr   r0, =0x20                                // switch to 4-bit mode
  bl    LCD_Nibble                               // send
  ldr   r0, =0x28                                // 4-bit, 2 lines, 5x8
  mov   r1, #0                                   // command
  bl    LCD_Byte                                 // send
  ldr   r0, =0x0c                                // display on, cursor off
  mov   r1, #0                                   // command
  bl    LCD_Byte                                 // send
  ldr   r0, =0x06                                // entry mode, increment
  mov   r1, #0                                   // command
  bl    LCD_Byte                                 // send
  ldr   r0, =0x01                                // clear display
  mov   r1, #0                                   // command
  bl    LCD_Byte                                 // send
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Print a NUL-terminated string on the LCD.
 *
 * @param   r0 - string address
 * @retval  None
 */
.global LCD_Print
.type LCD_Print, %function
LCD_Print:
.LCD_Print_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  mov   r4, r0                                   // string pointer
.LCD_Print_Loop:
  ldrb  r0, [r4]                                 // next character
  cmp   r0, #0                                   // end of string?
  beq   .LCD_Print_Done                          // yes -> finish
  mov   r1, #1                                   // data
  bl    LCD_Byte                                 // write character
  add   r4, r4, #1                               // advance
  b     .LCD_Print_Loop                          // loop
.LCD_Print_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

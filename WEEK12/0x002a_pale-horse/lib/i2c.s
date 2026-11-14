/**
 * FILE: i2c.s
 *
 * DESCRIPTION:
 * RP2350 I2C1 Master Functions.
 *
 * BRIEF:
 * Minimal blocking I2C1 master for the 1602 LCD backpack on GP2 (SDA) and
 * GP3 (SCL). Register offsets follow the DesignWare I2C block.
 *
 * NOTE:
 * The clock divider values assume CLK_PERI = 14.5 MHz. Validate bus timing
 * with the LCD attached before relying on it.
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
 * @brief   Initialize I2C1 master on GP2/GP3.
 *
 * @param   None
 * @retval  None
 */
.global I2C_Init
.type I2C_Init, %function
I2C_Init:
.I2C_Init_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.I2C_Init_Pins:
  ldr   r0, =IO_BANK0_BASE                       // GPIO function select base
  ldr   r1, =3                                   // FUNCSEL 3 -> I2C
  str   r1, [r0, #0x14]                          // GPIO2_CTRL (SDA)
  str   r1, [r0, #0x1c]                          // GPIO3_CTRL (SCL)
  ldr   r0, =PADS_BANK0_BASE                     // pad control base
  ldr   r1, =0x40                                // IE = 1
  str   r1, [r0, #0x0c]                          // PADS GPIO2
  str   r1, [r0, #0x10]                          // PADS GPIO3
.I2C_Init_Block:
  ldr   r0, =I2C1_BASE                           // I2C1 base
  ldr   r1, =0                                   // disable while configuring
  str   r1, [r0, #I2C_ENABLE]                    // IC_ENABLE = 0
  ldr   r1, =0x65                                // master, fast speed, restart
  str   r1, [r0, #I2C_CON]                       // IC_CON
  ldr   r1, =0x3c                                // SCL high count (100 kHz-ish)
  str   r1, [r0, #0x1c]                          // IC_FS_SCL_HCNT
  ldr   r1, =0x42                                // SCL low count
  str   r1, [r0, #0x20]                          // IC_FS_SCL_LCNT
  ldr   r1, =LCD_PCF_ADDR                        // LCD backpack address
  str   r1, [r0, #I2C_TAR]                       // IC_TAR
  ldr   r1, =1                                   // enable the controller
  str   r1, [r0, #I2C_ENABLE]                    // IC_ENABLE = 1
.I2C_Init_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Write one byte to the current target (with STOP).
 *
 * @param   r0 - byte to send
 * @retval  None
 */
.global I2C_Write
.type I2C_Write, %function
I2C_Write:
.I2C_Write_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r1, =I2C1_BASE                           // I2C1 base
  mov   r2, r0                                   // data byte
  ldr   r3, =0x200                               // STOP bit
  orr   r2, r2, r3                               // data | STOP
  str   r2, [r1, #I2C_DATA_CMD]                  // issue write
.I2C_Write_Wait:
  ldr   r2, [r1, #I2C_STATUS]                    // read status
  ldr   r3, =1                                   // TX FIFO not empty bit
  ands  r2, r2, r3                               // still busy?
  bne   .I2C_Write_Wait                          // wait until drained
.I2C_Write_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

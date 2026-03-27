/**
 * FILE: uart.s
 *
 * DESCRIPTION:
 * RP2350 UART Functions.
 * 
 * BRIEF:
 * Provides UART initialization, transmit, and receive functions for
 * UART0 on the RP2350.
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
 * @brief   Release UART0 from reset and wait until it is ready.
 *
 * @details Clears the UART0 reset bit in the Reset controller (RESETS->RESET)
 *          and polls the corresponding bit in RESETS->RESET_DONE until the
 *          UART0 block is no longer in reset. This ensures UART registers are
 *          accessible before configuring the peripheral.
 *
 * @param   None
 * @retval  None
 */
.global UART_Release_Reset
.type UART_Release_Reset, %function
UART_Release_Reset:
  ldr   r0, =RESETS_RESET                        // load RESETS->RESET address
  ldr   r1, [r0]                                 // read RESETS->RESET value
  bic   r1, r1, #(1<<26)                         // clear UART0 reset bit
  str   r1, [r0]                                 // write value back to RESETS->RESET
.UART_Release_Reset_Wait:
  ldr   r0, =RESETS_RESET_DONE                   // load RESETS->RESET_DONE address
  ldr   r1, [r0]                                 // read RESETS->RESET_DONE value
  tst   r1, #(1<<26)                             // test UART0 reset-done bit
  beq   .UART_Release_Reset_Wait                 // loop until UART0 is out of reset
  bx    lr                                       // return

/**
 * @brief   Initialize UART0 (pins, baud divisors, line control and enable).
 *
 * @details Configures IO_BANK0 pins 0 (TX) and 1 (RX) to the UART function
 *          and programs the corresponding pad controls in PADS_BANK0. It
 *          programs the integer and fractional baud divisors (UARTIBRD and
 *          UARTFBRD), configures UARTLCR_H for 8-bit transfers and FIFOs,
 *          and enables the UART (UARTCR: UARTEN + TXE + RXE).
 *          The routine assumes the UART0 base is available at the
 *          `UART0_BASE` symbol. The selected divisors (IBRD=6, FBRD=33) are
 *          chosen to match the expected peripheral clock; if your UART
 *          peripheral clock differs, adjust these values accordingly.
 *
 * @param   None
 * @retval  None
 */
.global UART_Init
.type UART_Init, %function
UART_Init:
  ldr   r0, =IO_BANK0_BASE                       // load IO_BANK0 base
  ldr   r1, =2                                   // FUNCSEL = 2 -> select UART function
  str   r1, [r0, #4]                             // write FUNCSEL to GPIO0_CTRL (pin0 -> TX)
  str   r1, [r0, #0x0c]                          // write FUNCSEL to GPIO1_CTRL (pin1 -> RX)
  ldr   r0, =PADS_BANK0_BASE                     // load PADS_BANK0 base
  add   r0, r0, #0x04                            // compute PAD[0] address (PADS + 0x04)
  ldr   r1, =0x04                                // pad config value for TX
  str   r1, [r0]                                 // write PAD0 config (TX pad)
  ldr   r0, =PADS_BANK0_BASE                     // load PADS_BANK0 base again
  add   r0, r0, #0x08                            // compute PAD[1] address (PADS + 0x08)
  ldr   r1, =0x40                                // pad config value for RX (pulldown/IE)
  str   r1, [r0]                                 // write PAD1 config (RX pad)
  ldr   r0, =UART0_BASE                          // load UART0 base address
  ldr   r1, =0                                   // prepare 0 to disable UARTCR
  str   r1, [r0, #0x30]                          // UARTCR = 0 (disable UART while configuring)
  ldr   r1, =6                                   // integer baud divisor (IBRD = 6)
  str   r1, [r0, #0x24]                          // UARTIBRD = 6 (integer baud divisor)
  ldr   r1, =33                                  // fractional baud divisor (FBRD = 33)
  str   r1, [r0, #0x28]                          // UARTFBRD = 33 (fractional baud divisor)
  ldr   r1, =112                                 // UARTLCR_H = 0x70 (FIFO enable + 8-bit)
  str   r1, [r0, #0x2c]                          // UARTLCR_H = 0x70
  ldr   r1, =3                                   // RXE/TXE mask
  lsl   r1, r1, #8                               // shift RXE/TXE into bit positions 8..9
  orr   r1, r1, #1                               // set UARTEN bit (bit 0)
  str   r1, [r0, #0x30]                          // UARTCR = enable (UARTEN + TXE + RXE)
  bx    lr                                       // return

/**
 * @brief   UART0 transmit (blocking).
 *
 * @details Waits for TX FIFO to be not full, then writes the lowest 8 bits of r0 to UART0.
 *          Data to send must be in r0 on entry.
 *
 * @param   r0: byte to transmit (lower 8 bits used)
 * @retval  None
 */
.global UART0_Out
.type UART0_Out, %function
UART0_Out:
.UART0_Out_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.UART0_Out_loop:
  ldr   r4, =UART0_BASE                          // base address for uart0 registers
  ldr   r5, [r4, #0x18]                          // read UART0 flag register UARTFR into r5
  ldr   r6, =32                                  // mask for bit 5, TX FIFO full (TXFF)
  ands  r5, r5, r6                               // isolate TXFF bit and set flags
  bne   .UART0_Out_loop                          // if TX FIFO is full, loop
  ldr   r6, =0xff                                // mask for the 8 lowest bits
  ands  r0, r0, r6                               // mask off upper bits of r0, keep lower 8 bits
  str   r0, [r4, #0]                             // write data to UARTDR
.UART0_Out_Pop_Registers:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   UART0 receive (blocking).
 *
 * @details Waits for RX FIFO to be not empty, then reads a byte from UART0 into r0.
 *
 * @param   None
 * @retval  r0: received byte (lower 8 bits valid)
 */
.global UART0_In
.type UART0_In, %function
UART0_In:
.UART0_In_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.UART0_In_loop:
  ldr   r4, =UART0_BASE                          // base address for uart0 registers
  ldr   r5, [r4, #0x18]                          // read UART0 flag register UARTFR into r5
  ldr   r6, =16                                  // mask for bit 4, RX FIFO empty RXFE
  ands  r5, r5, r6                               // isolate RXFE bit and set flags
  bne   .UART0_In_loop                           // if RX FIFO is empty, loop
  ldr   r0, [r4, #0]                             // load data from UARTDR into r0
.UART0_In_Pop_Registers:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

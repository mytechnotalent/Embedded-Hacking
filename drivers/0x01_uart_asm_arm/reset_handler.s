/**
 * FILE: reset_handler.s
 *
 * DESCRIPTION:
 * RP2350 Reset Handler.
 * 
 * BRIEF:
 * Entry point after reset. Performs initialization sequence including
 * stack setup, oscillator configuration, subsystem initialization, and
 * UART setup before branching to main application.
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
 * @brief   Reset handler for RP2350.
 *
 * @details Entry point after reset. Performs:
 *          - Stack initialization
 *          - Coprocessor enable
 *          - GPIO16 pad/function configuration
 *          - Branches to main() which contains the blink loop
 *
 * @param   None
 * @retval  None
 */
.global Reset_Handler                            // export Reset_Handler symbol
.type Reset_Handler, %function
Reset_Handler:
  bl    Init_Stack                               // initialize MSP/PSP and limits
  bl    Init_XOSC                                // initialize external crystal oscillator
  bl    Enable_XOSC_Peri_Clock                   // enable XOSC peripheral clock
  bl    Init_Subsystem                           // initialize subsystems
  bl    UART_Release_Reset                       // ensure UART0 out of reset
  bl    UART_Init                                // initialize UART0 (pins, baud, enable)
  bl    Enable_Coprocessor                       // enable CP0 coprocessor
  b     main                                     // branch to main loop
.size Reset_Handler, . - Reset_Handler

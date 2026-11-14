/**
 * FILE: reset.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-3 Reset Handler.
 *
 * BRIEF:
 * Application entry point. Initializes the stack and coprocessor, brings up
 * UART0, relocates the vector table into SRAM and installs it via VTOR,
 * then branches into the ghost stage in SRAM for authentication before
 * entering the console loop.
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
 * @brief   Stage-3 application reset handler.
 *
 * @details Stack -> coprocessor -> UART -> VTOR relocate -> ghost -> console.
 *
 * @param   None
 * @retval  None
 */
.global App_Reset
.type App_Reset, %function
App_Reset:
.App_Reset_Init:
  bl    Init_Stack                               // initialize MSP/PSP and limits
  bl    Init_XOSC                                // start the external crystal
  bl    Enable_XOSC_Peri_Clock                    // CLK_PERI sources XOSC
  bl    Init_Subsystem                           // release IO_BANK0 from reset
  bl    Enable_Coprocessor                       // enable CP0 coprocessor
  bl    UART_Release_Reset                       // release UART0 from reset
  bl    UART_Init                                // configure UART0 at 115200
.App_Reset_Relocate:
  ldr   r0, =APP_BASE                            // source vector table (flash)
  ldr   r1, =VTOR_RAM                            // destination vector table (SRAM)
  ldr   r2, [r0]                                 // initial SP
  str   r2, [r1]                                 // table[0]
  ldr   r2, [r0, #4]                             // reset vector
  str   r2, [r1, #4]                             // table[1]
  ldr   r2, =App_Default_Handler+1               // default handler (Thumb)
  mov   r3, #2                                   // start at slot 2
.App_Reset_Relocate_Loop:
  cmp   r3, #16                                  // filled all?
  beq   .App_Reset_Relocate_Done                 // yes -> install
  str   r2, [r1, r3, lsl #2]                      // table[r3] = default
  add   r3, r3, #1                               // next slot
  b     .App_Reset_Relocate_Loop                 // loop
.App_Reset_Relocate_Done:
  ldr   r0, =0xe000ed08                          // VTOR address
  ldr   r1, =VTOR_RAM                            // relocated table address
  str   r1, [r0]                                 // install VTOR
  dsb                                            // data sync barrier
  isb                                            // instruction sync barrier
.App_Reset_Ghost:
  ldr   r0, =GHOST_RAM                           // decrypted ghost stage in SRAM
  orr   r0, r0, #1                               // set Thumb bit for blx
  blx   r0                                       // execute the ghost from RAM
.App_Reset_Console:
  b     Console_Loop                             // enter the console
.size App_Reset, . - App_Reset

/**
 * @brief   Default exception handler.
 *
 * @retval  None
 */
.global App_Default_Handler
.type App_Default_Handler, %function
App_Default_Handler:
.App_Default_Handler_Halt:
  b     .App_Default_Handler_Halt                // halt forever

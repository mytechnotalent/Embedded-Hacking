/**
 * FILE: xosc.s
 *
 * DESCRIPTION:
 * RP2350 External Crystal Oscillator (XOSC) Functions.
 * 
 * BRIEF:
 * Provides functions to initialize the external crystal oscillator
 * and enable the XOSC peripheral clock.
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
 * @brief   Init XOSC and wait until it is ready.
 *
 * @details Configures and initializes the external crystal oscillator (XOSC).
 *          Waits for the XOSC to become stable before returning.
 *
 * @param   None
 * @retval  None
 */
.global Init_XOSC
.type Init_XOSC, %function
Init_XOSC:
  ldr   r0, =XOSC_STARTUP                        // load XOSC_STARTUP address
  ldr   r1, =0x00c4                              // set delay 50,000 cycles
  str   r1, [r0]                                 // store value into XOSC_STARTUP
  ldr   r0, =XOSC_CTRL                           // load XOSC_CTRL address
  ldr   r1, =0x00FABAA0                          // set 1_15MHz, freq range, actual 14.5MHz
  str   r1, [r0]                                 // store value into XOSC_CTRL
.Init_XOSC_Wait:
  ldr   r0, =XOSC_STATUS                         // load XOSC_STATUS address
  ldr   r1, [r0]                                 // read XOSC_STATUS value
  tst   r1, #(1<<31)                             // test STABLE bit
  beq   .Init_XOSC_Wait                          // wait until stable bit is set
  bx    lr                                       // return

/**
 * @brief   Enable XOSC peripheral clock.
 *
 * @details Sets the peripheral clock to use XOSC as its AUXSRC.
 *
 * @param   None
 * @retval  None
 */
.global Enable_XOSC_Peri_Clock
.type Enable_XOSC_Peri_Clock, %function
Enable_XOSC_Peri_Clock:
  ldr   r0, =CLK_PERI_CTRL                       // load CLK_PERI_CTRL address
  ldr   r1, [r0]                                 // read CLK_PERI_CTRL value
  orr   r1, r1, #(1<<11)                         // set ENABLE bit
  orr   r1, r1, #(4<<5)                          // set AUXSRC: XOSC_CLKSRC bit
  str   r1, [r0]                                 // store value into CLK_PERI_CTRL
  bx    lr                                       // return

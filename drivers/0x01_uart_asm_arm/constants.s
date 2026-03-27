/**
 * FILE: constants.s
 *
 * DESCRIPTION:
 * RP2350 Memory Addresses and Constants.
 * 
 * BRIEF:
 * Defines all memory-mapped register addresses and constants used
 * throughout the RP2350 driver.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 27, 2025
 * UPDATE DATE: November 27, 2025
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

/**
 * Memory addresses and constants.
 */
.equ STACK_TOP,                   0x20082000               // top of stack
.equ STACK_LIMIT,                 0x2007a000               // stack limit
.equ XOSC_BASE,                   0x40048000               // XOSC base address
.equ XOSC_CTRL,                   XOSC_BASE + 0x00        // XOSC control register
.equ XOSC_STATUS,                 XOSC_BASE + 0x04        // XOSC status register
.equ XOSC_STARTUP,                XOSC_BASE + 0x0c        // XOSC startup register
.equ PPB_BASE,                    0xe0000000               // PPB base address
.equ CPACR,                       PPB_BASE + 0x0ed88      // coprocessor access control
.equ CLOCKS_BASE,                 0x40010000               // clocks base address
.equ CLK_PERI_CTRL,               CLOCKS_BASE + 0x48      // peripheral clock control
.equ RESETS_BASE,                 0x40020000               // resets base address
.equ RESETS_RESET,                RESETS_BASE + 0x0       // reset register
.equ RESETS_RESET_CLEAR,          RESETS_BASE + 0x3000    // reset clear (atomic)
.equ RESETS_RESET_DONE,           RESETS_BASE + 0x8       // reset done register
.equ IO_BANK0_BASE,               0x40028000               // IO_BANK0 base address
.equ IO_BANK0_GPIO16_CTRL_OFFSET, 0x84                     // GPIO16 control offset
.equ PADS_BANK0_BASE,             0x40038000               // PADS_BANK0 base address
.equ PADS_BANK0_GPIO16_OFFSET,    0x44                     // GPIO16 pad offset
.equ UART0_BASE,                  0x40070000               // UART0 base address

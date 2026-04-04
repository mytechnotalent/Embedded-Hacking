/**
 * @file uart.c
 * @brief Implementation of the bare-metal UART0 driver
 * @author Kevin Thomas
 * @date 2025
 *
 * MIT License
 *
 * Copyright (c) 2025 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "uart.h"
#include "constants.h"

#define UART_BASE ((volatile uint32_t *)UART0_BASE)

/**
 * @brief Clear the UART0 reset bit in the reset controller
 *
 * @details Reads RESETS->RESET, clears bit 26 (UART0), and writes
 *          the value back.
 *
 * @param   None
 * @retval  None
 */
static void _uart_clear_reset_bit(void) {
    uint32_t value;

    // read RESETS->RESET value
    value = RESETS->RESET;
    // clear UART0 reset bit (bit 26)
    value &= ~(1U << RESETS_RESET_UART0_SHIFT);
    // write value back to RESETS->RESET
    RESETS->RESET = value;
}

/**
 * @brief Wait until the UART0 block is out of reset
 *
 * @details Polls RESETS->RESET_DONE until bit 26 (UART0) is set.
 *
 * @param   None
 * @retval  None
 */
static void _uart_wait_reset_done(void) {
    // loop until UART0 is out of reset
    while ((RESETS->RESET_DONE & (1U << RESETS_RESET_UART0_SHIFT)) == 0) {
    }
}

/**
 * @brief Configure GPIO pins 0 (TX) and 1 (RX) for UART function
 *
 * @details Sets IO_BANK0 FUNCSEL to UART (0x02) for both pins and
 *          programs the corresponding pad controls.
 *
 * @param   None
 * @retval  None
 */
static void _uart_configure_pins(void) {
    // FUNCSEL = 2 -> select UART function
    IO_BANK0->GPIO[0].CTRL = IO_BANK0_CTRL_FUNCSEL_UART;
    IO_BANK0->GPIO[1].CTRL = IO_BANK0_CTRL_FUNCSEL_UART;
    // pad config value for TX (pull/func recommended)
    PADS_BANK0->GPIO[0] = 0x04;
    // pad config value for RX (input enable)
    PADS_BANK0->GPIO[1] = 0x40;
}

/**
 * @brief Set UART0 baud rate divisors for 115200 baud at 14.5 MHz
 *
 * @details Programs UARTIBRD = 6 and UARTFBRD = 33 for the integer
 *          and fractional baud rate divisors respectively.
 *
 * @param   None
 * @retval  None
 */
static void _uart_set_baud(void) {
    // disable UART while configuring
    UART_BASE[UART_CR_OFFSET] = 0;
    // set integer baud divisor
    UART_BASE[UART_IBRD_OFFSET] = 6;
    // set fractional baud divisor
    UART_BASE[UART_FBRD_OFFSET] = 33;
}

/**
 * @brief Configure line control and enable UART0
 *
 * @details Sets 8-bit word length with FIFOs enabled, then enables
 *          UART0 with both TX and RX.
 *
 * @param   None
 * @retval  None
 */
static void _uart_enable(void) {
    // configure line control: FIFO enable + 8-bit
    UART_BASE[UART_LCR_H_OFFSET] = UART_LCR_H_8N1_FIFO;
    // enable UART with TX and RX enabled
    UART_BASE[UART_CR_OFFSET] = UART_CR_ENABLE;
}

void UART_Release_Reset(void) {
    _uart_clear_reset_bit();
    _uart_wait_reset_done();
}

void UART_Init(void) {
    _uart_configure_pins();
    _uart_set_baud();
    _uart_enable();
}

bool uart_driver_is_readable(void) {
    // test bit 4, RX FIFO empty (RXFE)
    return (UART_BASE[UART_FR_OFFSET] & UART_FR_RXFE_MASK) == 0;
}

char uart_driver_getchar(void) {
    // wait for RX FIFO to be not empty
    while (UART_BASE[UART_FR_OFFSET] & UART_FR_RXFE_MASK) {
    }
    // read data from UARTDR
    return (char)(UART_BASE[UART_DR_OFFSET] & 0xFF);
}

void uart_driver_putchar(char c) {
    // wait for TX FIFO to be not full
    while (UART_BASE[UART_FR_OFFSET] & UART_FR_TXFF_MASK) {
    }
    // write data to UARTDR
    UART_BASE[UART_DR_OFFSET] = (uint32_t)c;
}

void uart_driver_puts(const char *str) {
    while (*str) {
        uart_driver_putchar(*str++);
    }
}

char uart_driver_to_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        return (char)(c - 32);
    }
    return c;
}

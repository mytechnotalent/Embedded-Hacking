/**
 * @file main.c
 * @brief UART demonstration: echo received characters back in uppercase
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
 *
 * -----------------------------------------------------------------------------
 *
 * Demonstrates bare-metal UART0 using the uart driver (uart.h / uart.c).
 * Characters typed into a terminal via a USB-to-UART adapter are echoed
 * back in uppercase, illustrating full-duplex raw UART operation at the
 * register level without the Pico SDK.
 *
 * Wiring:
 *   GPIO0 (TX) -> USB-to-UART adapter RX
 *   GPIO1 (RX) -> USB-to-UART adapter TX
 *   GND        -> USB-to-UART adapter GND
 */

#include "uart.h"

/**
 * @brief Echo received characters back as uppercase
 *
 * @details Polls UART0 for incoming data. When a character is
 *          available, reads it, converts to uppercase, and echoes
 *          it back. Never returns.
 *
 * @param   None
 * @retval  None
 */
static void _echo_uppercase(void) {
    while (1) {
        if (uart_driver_is_readable()) {
            char c     = uart_driver_getchar();
            char upper = uart_driver_to_upper(c);
            uart_driver_putchar(upper);
        }
    }
}

/**
 * @brief Application entry point for the UART uppercase echo demo
 *
 * Prints two banner messages over UART0, then enters the echo loop.
 * UART0 hardware initialization is performed by Reset_Handler before
 * main() is called.
 *
 * @return int Does not return
 */
int main(void) {
    uart_driver_puts("UART driver ready (115200 8N1)\r\n");
    uart_driver_puts("Type characters to echo them back in UPPERCASE:\r\n");
    _echo_uppercase();
    return 0;
}

/**
 * @file 0x01_uart.c
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
 * Demonstrates hardware UART0 using the uart driver (uart.h / uart.c).
 * Characters typed into a terminal via a USB-to-UART adapter are echoed
 * back in uppercase, illustrating full-duplex raw UART operation without
 * going through the stdio layer.
 *
 * Wiring:
 *   GPIO0 (TX) -> USB-to-UART adapter RX
 *   GPIO1 (RX) -> USB-to-UART adapter TX
 *   GND        -> USB-to-UART adapter GND
 */

#include "pico/stdlib.h"
#include "uart.h"

#define UART_TX_PIN  0
#define UART_RX_PIN  1
#define UART_BAUD    115200

/**
 * @brief Application entry point for the UART uppercase echo demo
 *
 * Initializes UART0 and enters an infinite loop that reads incoming
 * characters, converts them to uppercase, and echoes them back.
 *
 * @return int Does not return
 */
int main(void) {
    uart_driver_init(UART_TX_PIN, UART_RX_PIN, UART_BAUD);
    uart_driver_puts("UART driver ready (115200 8N1)\r\n");
    uart_driver_puts("Type characters to echo them back in UPPERCASE:\r\n");
    while (true) {
        if (uart_driver_is_readable()) {
            char c     = uart_driver_getchar();
            char upper = uart_driver_to_upper(c);
            uart_driver_putchar(upper);
        }
    }
}

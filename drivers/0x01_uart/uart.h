/**
 * @file uart.h
 * @brief Header for hardware UART0 driver (raw TX/RX, GPIO 0/1)
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

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize hardware UART0 on the specified TX and RX GPIO pins
 *
 * Configures UART0 at the requested baud rate, sets the GPIO alternate
 * functions for TX and RX, and enables 8N1 framing. Must be called once
 * before using any other uart_driver_* functions.
 *
 * @param tx_pin   GPIO pin number to use as UART0 TX (typically 0)
 * @param rx_pin   GPIO pin number to use as UART0 RX (typically 1)
 * @param baud_rate Desired baud rate in bits per second (e.g. 115200)
 */
void uart_driver_init(uint32_t tx_pin, uint32_t rx_pin, uint32_t baud_rate);

/**
 * @brief Check whether a received character is waiting in the UART FIFO
 *
 * Returns immediately without blocking. Use this to poll for incoming
 * data before calling uart_driver_getchar().
 *
 * @return bool true if at least one byte is available to read, false otherwise
 */
bool uart_driver_is_readable(void);

/**
 * @brief Read one character from UART0 (blocking)
 *
 * Blocks until a byte arrives in the receive FIFO, then returns it.
 * Prefer pairing with uart_driver_is_readable() to avoid indefinite blocking.
 *
 * @return char The received character
 */
char uart_driver_getchar(void);

/**
 * @brief Transmit one character over UART0 (blocking)
 *
 * Waits until the transmit FIFO has space, then places the character into the
 * FIFO. Returns once the byte has been accepted by the hardware.
 *
 * @param c Character to transmit
 */
void uart_driver_putchar(char c);

/**
 * @brief Transmit a null-terminated string over UART0
 *
 * Calls uart_driver_putchar() for every character in the string up to and
 * not including the null terminator.
 *
 * @param str Pointer to the null-terminated ASCII string to send
 */
void uart_driver_puts(const char *str);

#endif // UART_H

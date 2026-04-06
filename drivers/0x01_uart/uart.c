/**
 * @file uart.c
 * @brief Implementation of the hardware UART0 driver
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
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

/** @brief Hardware UART instance used by this driver */
#define UART_INST uart0

/**
 * @brief Initialize hardware UART0 on the specified TX and RX GPIO pins
 *
 * @param tx_pin   GPIO pin number to use as UART0 TX
 * @param rx_pin   GPIO pin number to use as UART0 RX
 * @param baud_rate Desired baud rate in bits per second
 */
void uart_driver_init(uint32_t tx_pin, uint32_t rx_pin, uint32_t baud_rate) {
    uart_init(UART_INST, baud_rate);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
}

/**
 * @brief Check whether a received character is waiting in the UART FIFO
 *
 * @return bool true if at least one byte is available to read
 */
bool uart_driver_is_readable(void) {
    return uart_is_readable(UART_INST);
}

/**
 * @brief Read one character from UART0 (blocking)
 *
 * @return char The received character
 */
char uart_driver_getchar(void) {
    return (char)uart_getc(UART_INST);
}

/**
 * @brief Transmit one character over UART0 (blocking)
 *
 * @param c Character to transmit
 */
void uart_driver_putchar(char c) {
    uart_putc_raw(UART_INST, c);
}

/**
 * @brief Transmit a null-terminated string over UART0
 *
 * @param str Pointer to the string to transmit
 */
void uart_driver_puts(const char *str) {
    while (*str) {
        uart_putc_raw(UART_INST, *str++);
    }
}

/**
 * @brief Convert a character to uppercase
 *
 * @param c Character to convert
 * @return char Uppercase version if 'a'-'z', otherwise unchanged
 */
char uart_driver_to_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        return (char)(c - 32);
    }
    return c;
}

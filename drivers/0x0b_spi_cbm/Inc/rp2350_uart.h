/**
 * @file rp2350_uart.h
 * @brief UART0 driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Bare-metal UART0 driver supporting TX/RX on GPIO 0/1 at
 * 115200 baud (12 MHz XOSC clock).
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
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
#ifndef __RP2350_UART_H
#define __RP2350_UART_H

#include "rp2350.h"

/**
  * @brief  Release UART0 from reset and wait until ready.
  * @retval None
  */
void uart_release_reset(void);

/**
  * @brief  Initialize UART0 pins, baud rate, line control, and enable.
  * @retval None
  */
void uart_init(void);

/**
  * @brief  Check whether a received byte is waiting in the UART FIFO.
  * @retval bool true if at least one byte is available
  */
bool uart_is_readable(void);

/**
  * @brief  Read one character from UART0 (blocking).
  * @retval char the received character
  */
char uart_getchar(void);

/**
  * @brief  Transmit one character over UART0 (blocking).
  * @param  c character to transmit
  * @retval None
  */
void uart_putchar(char c);

/**
  * @brief  Transmit a null-terminated string over UART0.
  * @param  str pointer to the string to send
  * @retval None
  */
void uart_puts(const char *str);

/**
  * @brief  Convert a lowercase ASCII character to uppercase.
  * @param  c input character
  * @retval char uppercase equivalent or original character
  */
char uart_to_upper(char c);

#endif /* __RP2350_UART_H */

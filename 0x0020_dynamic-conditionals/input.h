/**
 * MIT License
 * 
 * Copyright (c) 2024 My Techno Talent
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

/** @file input.h
 * 
 *  @brief Constant and function prototypes for the input driver.
 * 
 *  This contains the constants and function prototypes for the input driver which
 *  will handle UART configuration in addition to capturing a character from the 
 *  UART terminal.
 *   
 *  @author Kevin Thomas
 *  @date   04/06/2024
*/

#ifndef _INPUT_H
#define _INPUT_H

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

/**
 * @brief Initializes UART0 with specified baud rate and pin configurations.
 *
 * This function initializes UART0 with the specified baud rate and configures the GPIO pins
 * for UART transmission (TX) and reception (RX).
 *
 * @param  None
 * @return None
 */
void uart0_init(void);

/**
 * @brief Handles a UART received character.
 *
 * This function checks if data is available to read from UART, and if so,
 * it reads and returns the received character. If no data is available,
 * it returns 0.
 *
 * @param  None
 * @return The received character if available, or 0 if no data is available.
 */
uint8_t on_uart_rx(void);

#endif /* _INPUT_H */

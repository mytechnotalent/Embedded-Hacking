/**
 * FILE: usart.h
 *
 * DESCRIPTION: This file contains the usart function definitions utilizing the STM32F401CC6 microcontroller.
 *
 * AUTHOR: Kevin Thomas
 * DATE: October 6, 2023
 */

#ifndef USART_H
#define USART_H

#include <stdint.h>

#include "stm32f4xx.h"

#define DEFAULT_CLK_SPEED 16000000U

extern int __io_putchar(int ch) __attribute__((weak));

/**
 * @brief  Sets the baud rate for a given USART peripheral.
 *
 *         This function calculates and sets the baud rate register (BRR) for
 *         the specified USART peripheral based on the provided peripheral clock
 *         speed and desired baud rate.
 *
 * @param  USARTx: Pointer to the USART peripheral (e.g., USART1, USART2, etc.).
 * @param  periph_clk: The frequency of the peripheral clock in Hertz.
 * @param  baud_rate: The desired baud rate for communication.
 * @retval None
 */
static void set_baud_rate(USART_TypeDef *USARTx,  uint32_t periph_clk, uint32_t baud_rate);

/**
 * @brief  Initializes USART2 for transmission on PA2 pin and receive on PA3 pin.
 *
 *         This function configures the necessary settings for USART2 to enable
 *         transmission on pin PA2 and receive on PA3 pin. It includes configuring 
 *         GPIO, setting alternate function mode, configuring the USART clock, setting the
 *         baud rate, and enabling the transmitter.
 *
 * @param  None
 * @retval None
 */
void usart2_init(void);

/**
 * @brief  Writes a single byte to USART2 for transmission.
 *
 *         This function writes a single byte to the USART2  data
 *         register for transmission. It ensures that the data register
 *         is empty before writing the byte.
 *
 * @param  char_: The byte to be transmitted.
 * @retval None
 */
void usart2_write_char(uint8_t char_);

/**
 * @brief  Reads a single byte from USART2.
 *
 *         This function reads a single byte from the USART2 data
 *         register. It ensures that the data register is not empty
 *         before reading the byte.
 *
 * @param  None
 * @retval The byte read from the data register.
 */
char usart2_read_char(void);

/**
 * @brief  Reads a string of characters from input and stores it in the buffer.
 *
 *         This function reads characters from input and stores them in the
 *         provided buffer until either the buffer is full (reaching
 *         buffer_size - 1 characters) or a newline character is encountered.
 *         The input is null-terminated, ensuring proper string termination in
 *         the buffer.
 *
 * @param  buffer: Pointer to the buffer where the input characters are stored.
 * @param  buffer_size: The size of the buffer, including space for the null-terminator.
 * @retval None
 */
void input(char *buffer, size_t buffer_size);

#endif // USART_H

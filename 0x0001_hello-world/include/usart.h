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
 * @brief  Initializes USART2 for transmission on PA2 pin.
 *
 *         This function configures the necessary settings for USART2 to enable
 *         transmission on pin PA2. It includes configuring GPIO, setting
 *         alternate function mode, configuring the USART clock, setting the
 *         baud rate, and enabling the transmitter.
 *
 * @param  None
 * @retval None
 */
void usart2_tx_init(void);

/**
 * @brief  Writes a single byte to USART2 for transmission.
 *
 *         This function writes a single byte to the USART2 transmit data
 *         register for transmission. It ensures that the transmit data register
 *         is empty before writing the byte.
 *
 * @param  ch: The byte to be transmitted.
 * @retval None
 */
void usart2_write(uint8_t ch);

#endif // USART_H

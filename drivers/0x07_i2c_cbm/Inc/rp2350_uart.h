/**
  ******************************************************************************
  * @file    rp2350_uart.h
  * @author  Kevin Thomas
  * @brief   UART0 driver header for RP2350.
  *
  *          Bare-metal UART0 driver supporting TX/RX on GPIO 0/1 at
  *          115200 baud (14.5 MHz XOSC clock).
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
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

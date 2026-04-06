/**
  ******************************************************************************
  * @file    rp2350_uart.c
  * @author  Kevin Thomas
  * @brief   UART0 driver implementation for RP2350.
  *
  *          Configures UART0 on GPIO 0 (TX) and GPIO 1 (RX) at 115200
  *          baud using the 12 MHz XOSC clock.
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

#include "rp2350_uart.h"

#define UART_BASE ((volatile uint32_t *) UART0_BASE)

/**
  * @brief  Clear the UART0 reset bit in the reset controller.
  * @retval None
  */
static void _uart_clear_reset_bit(void)
{
  uint32_t value;
  value = RESETS->RESET;
  value &= ~(1U << RESETS_RESET_UART0_SHIFT);
  RESETS->RESET = value;
}

/**
  * @brief  Wait until the UART0 block is out of reset.
  * @retval None
  */
static void _uart_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_UART0_SHIFT)) == 0) {
  }
}

/**
  * @brief  Configure GPIO pins 0 (TX) and 1 (RX) for UART function.
  * @retval None
  */
static void _uart_configure_pins(void)
{
  IO_BANK0->GPIO[0].CTRL = IO_BANK0_CTRL_FUNCSEL_UART;
  IO_BANK0->GPIO[1].CTRL = IO_BANK0_CTRL_FUNCSEL_UART;
  PADS_BANK0->GPIO[0] = 0x04;
  PADS_BANK0->GPIO[1] = 0x40;
}

/**
  * @brief  Set UART0 baud rate divisors for 115200 at 12 MHz.
  * @retval None
  */
static void _uart_set_baud(void)
{
  UART_BASE[UART_CR_OFFSET] = 0;
  UART_BASE[UART_IBRD_OFFSET] = 6;
  UART_BASE[UART_FBRD_OFFSET] = 33;
}

/**
  * @brief  Configure line control and enable UART0.
  * @retval None
  */
static void _uart_enable(void)
{
  UART_BASE[UART_LCR_H_OFFSET] = UART_LCR_H_8N1_FIFO;
  UART_BASE[UART_CR_OFFSET] = UART_CR_ENABLE;
}

void uart_release_reset(void)
{
  _uart_clear_reset_bit();
  _uart_wait_reset_done();
}

void uart_init(void)
{
  _uart_configure_pins();
  _uart_set_baud();
  _uart_enable();
}

bool uart_is_readable(void)
{
  return (UART_BASE[UART_FR_OFFSET] & UART_FR_RXFE_MASK) == 0;
}

char uart_getchar(void)
{
  while (UART_BASE[UART_FR_OFFSET] & UART_FR_RXFE_MASK) {
  }
  return (char)(UART_BASE[UART_DR_OFFSET] & 0xFF);
}

void uart_putchar(char c)
{
  while (UART_BASE[UART_FR_OFFSET] & UART_FR_TXFF_MASK) {
  }
  UART_BASE[UART_DR_OFFSET] = (uint32_t)c;
}

void uart_puts(const char *str)
{
  while (*str) {
    uart_putchar(*str++);
  }
}

char uart_to_upper(char c)
{
  if (c >= 'a' && c <= 'z')
    return (char)(c - 32);
  return c;
}

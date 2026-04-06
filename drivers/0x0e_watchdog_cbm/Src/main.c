/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   Watchdog feed demonstration.
  *
  *          Enables the watchdog with a 3-second timeout and feeds it
  *          every 1 second. Reports whether the system booted from a
  *          watchdog reset or a normal power-on.
  *
  *          Wiring:
  *            GPIO0 -> UART TX (USB-to-UART adapter RX)
  *            GPIO1 -> UART RX (USB-to-UART adapter TX)
  *            No external components required
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

#include "rp2350_watchdog.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/**
  * @brief  Print the reset reason over UART.
  * @retval None
  */
static void _print_reset_reason(void)
{
  if (watchdog_caused_reboot())
    uart_puts("System rebooted by watchdog timeout\r\n");
  else
    uart_puts("Normal power-on reset\r\n");
}

/**
  * @brief  Feed the watchdog, report over UART, and delay 1 second.
  * @retval None
  */
static void _feed_and_report(void)
{
  watchdog_feed();
  uart_puts("Watchdog fed\r\n");
  delay_ms(1000);
}

int main(void)
{
  _print_reset_reason();
  watchdog_enable(3000);
  uart_puts("Watchdog enabled (3s timeout). Feeding every 1s...\r\n");
  while (1)
    _feed_and_report();
}

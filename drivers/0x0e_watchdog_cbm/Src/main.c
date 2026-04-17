/**
 * @file main.c
 * @brief Watchdog feed demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Enables the watchdog with a 3-second timeout and feeds it
 * every 1 second. Reports whether the system booted from a
 * watchdog reset or a normal power-on.
 *
 * Wiring:
 *  GPIO0 -> UART TX (USB-to-UART adapter RX)
 *  GPIO1 -> UART RX (USB-to-UART adapter TX)
 *  No external components required
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
#include "rp2350_watchdog.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/**
  * @brief  Print the reset reason over UART.
  * @retval None
  */
static void print_reset_reason(void)
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
static void feed_and_report(void)
{
  watchdog_feed();
  uart_puts("Watchdog fed\r\n");
  delay_ms(1000);
}

int main(void)
{
  print_reset_reason();
  watchdog_enable(3000);
  uart_puts("Watchdog enabled (3s timeout). Feeding every 1s...\r\n");
  while (1)
    feed_and_report();
}

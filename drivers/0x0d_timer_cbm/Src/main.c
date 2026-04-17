/**
 * @file main.c
 * @brief Repeating timer alarm demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Configures TIMER0 alarm 0 to fire a callback every 1 second.
 * The callback prints "Timer heartbeat" over UART. The main
 * loop idles with WFI.
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
#include "rp2350_timer.h"
#include "rp2350_uart.h"

/**
  * @brief  Heartbeat callback invoked by TIMER0 alarm 0 IRQ.
  * @retval None
  */
static void heartbeat(void)
{
  uart_puts("Timer heartbeat\r\n");
}

int main(void)
{
  uart_puts("Timer alarm demo initialized\r\n");
  timer_alarm_start(1000, heartbeat);
  while (1)
    __asm__ volatile ("wfi");
}

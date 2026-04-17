/**
 * @file main.c
 * @brief UART demonstration: echo received characters in uppercase.
 * @author Kevin Thomas
 * @date 2026
 *
 * Demonstrates hardware UART0 using the bare-metal UART driver.
 * Characters typed into a terminal via a USB-to-UART adapter
 * are echoed back in uppercase.
 *
 * Wiring:
 *  GPIO0 (TX) -> USB-to-UART adapter RX
 *  GPIO1 (RX) -> USB-to-UART adapter TX
 *  GND        -> USB-to-UART adapter GND
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
#include "rp2350_uart.h"

int main(void)
{
  uart_puts("UART driver ready (115200 8N1)\r\n");
  uart_puts("Type characters to echo them back in UPPERCASE:\r\n");
  while (1) 
  {
    if (uart_is_readable()) 
    {
      char c = uart_getchar();
      char upper = uart_to_upper(c);
      uart_putchar(upper);
    }
  }
}

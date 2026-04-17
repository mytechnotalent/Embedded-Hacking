/**
 * @file main.c
 * @brief Multicore FIFO messaging demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Core 0 launches core 1, then sends an incrementing counter
 * through the SIO inter-processor FIFO. Core 1 returns the
 * value plus one. Both values are printed over UART every
 * second.
 *
 * Wiring:
 *  GPIO0 -> UART TX (USB-to-UART adapter RX)
 *  GPIO1 -> UART RX (USB-to-UART adapter TX)
 *  No external wiring required (dual-core on-chip)
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
#include "rp2350_multicore.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/**
  * @brief  Extract decimal digits from a value in reverse order.
  * @param  value unsigned integer to convert
  * @param  tmp   output buffer for reversed digits (min 11 bytes)
  * @retval int   number of digits written
  */
static int extract_digits(uint32_t value, char *tmp)
{
  int i = 0;
  if (value == 0)
    tmp[i++] = '0';
  while (value > 0) 
  {
    tmp[i++] = '0' + (char)(value % 10);
    value /= 10;
  }
  return i;
}

/**
  * @brief  Reverse-copy a digit buffer into a null-terminated string.
  * @param  tmp source buffer with reversed digits
  * @param  len number of digits in source
  * @param  buf destination buffer (must hold len + 1 bytes)
  * @retval None
  */
static void reverse_copy(const char *tmp, int len, char *buf)
{
  int j = 0;
  while (len > 0)
    buf[j++] = tmp[--len];
  buf[j] = '\0';
}

/**
  * @brief  Convert a uint32_t to a decimal null-terminated string.
  * @param  value unsigned integer to convert
  * @param  buf   output buffer (min 12 bytes)
  * @retval None
  */
static void uint_to_str(uint32_t value, char *buf)
{
  char tmp[11];
  int len = extract_digits(value, tmp);
  reverse_copy(tmp, len, buf);
}

/**
  * @brief  Print a labelled decimal value over UART.
  * @param  label text label to print before the value
  * @param  value unsigned integer to print
  * @retval None
  */
static void print_labeled_value(const char *label, uint32_t value)
{
  char buf[12];
  uart_puts(label);
  uint_to_str(value, buf);
  uart_puts(buf);
}

/**
  * @brief  Print a full sent/returned line over UART.
  * @param  sent      value sent by core 0
  * @param  received  value returned by core 1
  * @retval None
  */
static void print_counter_line(uint32_t sent, uint32_t received)
{
  print_labeled_value("core0 sent: ", sent);
  print_labeled_value(", core1 returned: ", received);
  uart_puts("\r\n");
}

/**
  * @brief  Core 1 entry point: receive a value and return it plus one.
  * @retval None (does not return)
  */
static void core1_main(void)
{
  while (1) 
  {
    uint32_t value = multicore_fifo_pop();
    multicore_fifo_push(value + 1);
  }
}

/**
  * @brief  Send the counter to core 1, print the round-trip, delay.
  * @param  counter pointer to the running counter (post-incremented)
  * @retval None
  */
static void send_and_print(uint32_t *counter)
{
  multicore_fifo_push(*counter);
  uint32_t response = multicore_fifo_pop();
  print_counter_line(*counter, response);
  (*counter)++;
  delay_ms(1000);
}

int main(void)
{
  uint32_t counter = 0;
  multicore_launch(core1_main);
  uart_puts("Multicore FIFO demo initialized\r\n");
  while (1)
    send_and_print(&counter);
}

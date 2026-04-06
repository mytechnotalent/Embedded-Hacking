/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   Multicore FIFO messaging demonstration.
  *
  *          Core 0 launches core 1, then sends an incrementing counter
  *          through the SIO inter-processor FIFO. Core 1 returns the
  *          value plus one. Both values are printed over UART every
  *          second.
  *
  *          Wiring:
  *            GPIO0 -> UART TX (USB-to-UART adapter RX)
  *            GPIO1 -> UART RX (USB-to-UART adapter TX)
  *            No external wiring required (dual-core on-chip)
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

#include "rp2350_multicore.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/**
  * @brief  Extract decimal digits from a value in reverse order.
  * @param  value unsigned integer to convert
  * @param  tmp   output buffer for reversed digits (min 11 bytes)
  * @retval int   number of digits written
  */
static int _extract_digits(uint32_t value, char *tmp)
{
  int i = 0;
  if (value == 0)
    tmp[i++] = '0';
  while (value > 0) {
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
static void _reverse_copy(const char *tmp, int len, char *buf)
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
static void _uint_to_str(uint32_t value, char *buf)
{
  char tmp[11];
  int len = _extract_digits(value, tmp);
  _reverse_copy(tmp, len, buf);
}

/**
  * @brief  Print a labelled decimal value over UART.
  * @param  label text label to print before the value
  * @param  value unsigned integer to print
  * @retval None
  */
static void _print_labeled_value(const char *label, uint32_t value)
{
  char buf[12];
  uart_puts(label);
  _uint_to_str(value, buf);
  uart_puts(buf);
}

/**
  * @brief  Print a full sent/returned line over UART.
  * @param  sent      value sent by core 0
  * @param  received  value returned by core 1
  * @retval None
  */
static void _print_counter_line(uint32_t sent, uint32_t received)
{
  _print_labeled_value("core0 sent: ", sent);
  _print_labeled_value(", core1 returned: ", received);
  uart_puts("\r\n");
}

/**
  * @brief  Core 1 entry point: receive a value and return it plus one.
  * @retval None (does not return)
  */
static void _core1_main(void)
{
  while (1) {
    uint32_t value = multicore_fifo_pop();
    multicore_fifo_push(value + 1);
  }
}

/**
  * @brief  Send the counter to core 1, print the round-trip, delay.
  * @param  counter pointer to the running counter (post-incremented)
  * @retval None
  */
static void _send_and_print(uint32_t *counter)
{
  multicore_fifo_push(*counter);
  uint32_t response = multicore_fifo_pop();
  _print_counter_line(*counter, response);
  (*counter)++;
  delay_ms(1000);
}

/**
  * @brief  Application entry point for the multicore FIFO demo.
  * @retval int does not return
  */
int main(void)
{
  uint32_t counter = 0;
  multicore_launch(_core1_main);
  uart_puts("Multicore FIFO demo initialized\r\n");
  while (1)
    _send_and_print(&counter);
}

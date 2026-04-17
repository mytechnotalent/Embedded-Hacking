/**
 * @file main.c
 * @brief ADC demonstration: potentiometer voltage and chip temperature.
 * @author Kevin Thomas
 * @date 2026
 *
 * Demonstrates 12-bit ADC using the bare-metal ADC driver. Reads
 * ADC channel 0 (GPIO26) and reports the voltage in millivolts
 * alongside the on-chip temperature sensor reading every 500 ms
 * over UART.
 *
 * Wiring:
 *  GPIO0  -> UART TX (USB-to-UART adapter RX)
 *  GPIO1  -> UART RX (USB-to-UART adapter TX)
 *  GPIO26 -> Wiper of a 10 kohm potentiometer
 *  3.3V   -> One end of the potentiometer
 *  GND    -> Other end of the potentiometer
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
#include "rp2350_adc.h"
#include "rp2350_uart.h"
#include "rp2350_xosc.h"
#include "rp2350_delay.h"

/** @brief Delay between ADC samples in milliseconds */
#define SAMPLE_DELAY_MS 500

/**
  * @brief  Reverse a character buffer in place.
  * @param  buf pointer to the buffer
  * @param  len number of characters to reverse
  * @retval None
  */
static void reverse(char *buf, uint8_t len)
{
  for (uint8_t i = 0; i < len / 2; i++) 
  {
    char tmp = buf[i];
    buf[i] = buf[len - 1 - i];
    buf[len - 1 - i] = tmp;
  }
}

/**
  * @brief  Print an unsigned 32-bit integer as a decimal string over UART.
  * @param  val value to print
  * @retval None
  */
static void print_uint32(uint32_t val)
{
  char buf[11];
  uint8_t len = 0;
  do { buf[len++] = (char)('0' + val % 10); val /= 10; } while (val > 0);
  reverse(buf, len);
  buf[len] = '\0';
  uart_puts(buf);
}

/**
  * @brief  Print a signed temperature in tenths of degrees as XX.X over UART.
  * @param  tenths temperature in tenths of degrees Celsius
  * @retval None
  */
static void print_temp(int32_t tenths)
{
  if (tenths < 0) { uart_puts("-"); tenths = -tenths; }
  print_uint32((uint32_t)(tenths / 10));
  char frac[2] = { (char)('0' + tenths % 10), '\0' };
  uart_puts(".");
  uart_puts(frac);
}

/**
  * @brief  Print ADC voltage and chip temperature readings over UART.
  * @retval None
  */
static void print_readings(void)
{
  uint32_t mv = adc_read_mv();
  int32_t temp = adc_read_temp_tenths();
  uart_puts("ADC0: ");
  print_uint32(mv);
  uart_puts(" mV  |  Chip temp: ");
  print_temp(temp);
  uart_puts(" C\r\n");
}

int main(void)
{
  xosc_enable_adc_clk();
  adc_release_reset();
  adc_init();
  while (1) 
  {
    print_readings();
    delay_ms(SAMPLE_DELAY_MS);
  }
}

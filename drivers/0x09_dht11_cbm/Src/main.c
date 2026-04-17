/**
 * @file main.c
 * @brief DHT11 temperature and humidity sensor demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Reads the DHT11 sensor on GPIO4 every 2 seconds, printing
 * humidity and temperature over UART. Reports read failures
 * for wiring diagnostics.
 *
 * Wiring:
 *  GPIO0  -> UART TX (USB-to-UART adapter RX)
 *  GPIO1  -> UART RX (USB-to-UART adapter TX)
 *  GPIO4  -> DHT11 DATA (10k pull-up to 3.3V recommended)
 *  3.3V   -> DHT11 VCC
 *  GND    -> DHT11 GND
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
#include "rp2350_dht11.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"
#include "rp2350_xosc.h"

/**
  * @brief  Minimum polling interval for the DHT11 sensor in milliseconds.
  */
/** @brief Interval between DHT11 sensor reads in milliseconds */
#define DHT11_POLL_MS  2000U

/**
  * @brief  Convert an unsigned integer (0-255) to a decimal string.
  * @param  value number to convert
  * @param  buf   output buffer (at least 4 bytes)
  * @retval None
  */
static void uint_to_str(uint8_t value, char *buf)
{
  uint8_t idx = 0;
  if (value >= 100)
    buf[idx++] = (char)('0' + value / 100);
  if (value >= 10)
    buf[idx++] = (char)('0' + (value / 10) % 10);
  buf[idx++] = (char)('0' + value % 10);
  buf[idx] = '\0';
}

/**
  * @brief  Print a successful DHT11 reading over UART.
  * @param  humidity    humidity percentage (integer)
  * @param  temperature temperature in Celsius (integer)
  * @retval None
  */
static void print_reading(uint8_t humidity, uint8_t temperature)
{
  char buf[4];
  uart_puts("Humidity: ");
  uint_to_str(humidity, buf);
  uart_puts(buf);
  uart_puts("%  Temperature: ");
  uint_to_str(temperature, buf);
  uart_puts(buf);
  uart_puts(" C\r\n");
}

/**
  * @brief  Print a read failure message over UART.
  * @retval None
  */
static void print_failure(void)
{
  uart_puts("DHT11 read failed - check wiring on GPIO4\r\n");
}

/**
  * @brief  Initialize clocks, timer, DHT11, and announce over UART.
  * @retval None
  */
static void dht11_setup(void)
{
  uint8_t dummy_h;
  uint8_t dummy_t;
  xosc_set_clk_ref();
  dht11_timer_release_reset();
  dht11_timer_start_tick();
  dht11_init();
  delay_ms(DHT11_POLL_MS);
  dht11_read(&dummy_h, &dummy_t);
  delay_ms(DHT11_POLL_MS);
  uart_puts("DHT11 driver initialized on GPIO4\r\n");
}

/**
  * @brief  Read the sensor once and print results or failure.
  * @retval None
  */
static void poll_sensor(void)
{
  uint8_t humidity;
  uint8_t temperature;
  if (dht11_read(&humidity, &temperature))
    print_reading(humidity, temperature);
  else
    print_failure();
  delay_ms(DHT11_POLL_MS);
}

int main(void)
{
  dht11_setup();
  while (1)
    poll_sensor();
}

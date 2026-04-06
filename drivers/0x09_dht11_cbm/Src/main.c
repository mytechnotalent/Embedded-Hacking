/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   DHT11 temperature and humidity sensor demonstration.
  *
  *          Reads the DHT11 sensor on GPIO4 every 2 seconds, printing
  *          humidity and temperature over UART. Reports read failures
  *          for wiring diagnostics.
  *
  *          Wiring:
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO4  -> DHT11 DATA (10k pull-up to 3.3V recommended)
  *            3.3V   -> DHT11 VCC
  *            GND    -> DHT11 GND
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

#include "rp2350_dht11.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"
#include "rp2350_xosc.h"

/**
  * @brief  Minimum polling interval for the DHT11 sensor in milliseconds.
  */
#define DHT11_POLL_MS  2000U

/**
  * @brief  Convert an unsigned integer (0-255) to a decimal string.
  * @param  value number to convert
  * @param  buf   output buffer (at least 4 bytes)
  * @retval None
  */
static void _uint_to_str(uint8_t value, char *buf)
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
static void _print_reading(uint8_t humidity, uint8_t temperature)
{
  char buf[4];
  uart_puts("Humidity: ");
  _uint_to_str(humidity, buf);
  uart_puts(buf);
  uart_puts("%  Temperature: ");
  _uint_to_str(temperature, buf);
  uart_puts(buf);
  uart_puts(" C\r\n");
}

/**
  * @brief  Print a read failure message over UART.
  * @retval None
  */
static void _print_failure(void)
{
  uart_puts("DHT11 read failed - check wiring on GPIO4\r\n");
}

/**
  * @brief  Initialize clocks, timer, DHT11, and announce over UART.
  * @retval None
  */
static void _dht11_setup(void)
{
  xosc_set_clk_ref();
  dht11_timer_release_reset();
  dht11_timer_start_tick();
  dht11_init();
  uart_puts("DHT11 driver initialized on GPIO4\r\n");
}

/**
  * @brief  Read the sensor once and print results or failure.
  * @retval None
  */
static void _poll_sensor(void)
{
  uint8_t humidity;
  uint8_t temperature;
  if (dht11_read(&humidity, &temperature))
    _print_reading(humidity, temperature);
  else
    _print_failure();
  delay_ms(DHT11_POLL_MS);
}

/**
  * @brief  Application entry point for the DHT11 sensor demo.
  * @retval int does not return
  */
int main(void)
{
  _dht11_setup();
  while (1)
    _poll_sensor();
}

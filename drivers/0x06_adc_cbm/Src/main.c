/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   ADC demonstration: potentiometer voltage and chip temperature.
  *
  *          Demonstrates 12-bit ADC using the bare-metal ADC driver. Reads
  *          ADC channel 0 (GPIO26) and reports the voltage in millivolts
  *          alongside the on-chip temperature sensor reading every 500 ms
  *          over UART.
  *
  *          Wiring:
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO26 -> Wiper of a 10 kohm potentiometer
  *            3.3V   -> One end of the potentiometer
  *            GND    -> Other end of the potentiometer
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

#include "rp2350_adc.h"
#include "rp2350_uart.h"
#include "rp2350_xosc.h"
#include "rp2350_delay.h"

#define SAMPLE_DELAY_MS 500

/**
  * @brief  Reverse a character buffer in place.
  * @param  buf pointer to the buffer
  * @param  len number of characters to reverse
  * @retval None
  */
static void _reverse(char *buf, uint8_t len)
{
  for (uint8_t i = 0; i < len / 2; i++) {
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
static void _print_uint32(uint32_t val)
{
  char buf[11];
  uint8_t len = 0;
  do { buf[len++] = (char)('0' + val % 10); val /= 10; } while (val > 0);
  _reverse(buf, len);
  buf[len] = '\0';
  uart_puts(buf);
}

/**
  * @brief  Print a signed temperature in tenths of degrees as XX.X over UART.
  * @param  tenths temperature in tenths of degrees Celsius
  * @retval None
  */
static void _print_temp(int32_t tenths)
{
  if (tenths < 0) { uart_puts("-"); tenths = -tenths; }
  _print_uint32((uint32_t)(tenths / 10));
  char frac[2] = { (char)('0' + tenths % 10), '\0' };
  uart_puts(".");
  uart_puts(frac);
}

/**
  * @brief  Print ADC voltage and chip temperature readings over UART.
  * @retval None
  */
static void _print_readings(void)
{
  uint32_t mv = adc_read_mv();
  int32_t temp = adc_read_temp_tenths();
  uart_puts("ADC0: ");
  _print_uint32(mv);
  uart_puts(" mV  |  Chip temp: ");
  _print_temp(temp);
  uart_puts(" C\r\n");
}

int main(void)
{
  xosc_enable_adc_clk();
  adc_release_reset();
  adc_init();
  while (1) {
    _print_readings();
    delay_ms(SAMPLE_DELAY_MS);
  }
}

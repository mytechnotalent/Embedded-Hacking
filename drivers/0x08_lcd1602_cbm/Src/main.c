/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   LCD1602 demonstration: static title and live counter display.
  *
  *          Drives a 16x2 HD44780 LCD via a PCF8574 I2C backpack. Line 0
  *          shows a static title ("Reverse Eng.") and line 1 displays a
  *          counter that increments every second. The counter value is
  *          also printed over UART for debugging.
  *
  *          Wiring:
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO2  -> PCF8574 backpack SDA (4.7 kohm pull-up to 3.3 V)
  *            GPIO3  -> PCF8574 backpack SCL (4.7 kohm pull-up to 3.3 V)
  *            3.3V   -> PCF8574 backpack VCC
  *            GND    -> PCF8574 backpack GND
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

#include "rp2350_i2c.h"
#include "rp2350_lcd1602.h"
#include "rp2350_uart.h"
#include "rp2350_xosc.h"
#include "rp2350_delay.h"

#define COUNT_DELAY_MS 1000

/**
  * @brief  Convert an unsigned 32-bit integer to a decimal string.
  *
  *         Writes at most 10 decimal digits plus a null terminator
  *         into the provided buffer. The buffer must be at least
  *         11 bytes.
  *
  * @param  val value to convert
  * @param  buf destination buffer (minimum 11 bytes)
  * @retval None
  */
static void _uint_to_str(uint32_t val, char *buf)
{
  char tmp[11];
  int i = 0;
  if (val == 0) {
    buf[0] = '0';
    buf[1] = '\0';
    return;
  }
  while (val > 0) {
    tmp[i++] = (char)('0' + (val % 10));
    val /= 10;
  }
  for (int j = 0; j < i; j++)
    buf[j] = tmp[i - 1 - j];
  buf[i] = '\0';
}

/**
  * @brief  Format "Count: " followed by a right-justified counter value.
  *
  *         Writes the formatted string into a 17-byte buffer and pads
  *         with spaces to fill the 16-column display width.
  *
  * @param  count current counter value
  * @param  buf   destination buffer (minimum 17 bytes)
  * @retval None
  */
static void _format_counter(uint32_t count, char *buf)
{
  char num[11];
  _uint_to_str(count, num);
  const char *prefix = "Count: ";
  int i = 0;
  while (prefix[i]) { buf[i] = prefix[i]; i++; }
  int j = 0;
  while (num[j]) buf[i++] = num[j++];
  while (i < 16) buf[i++] = ' ';
  buf[i] = '\0';
}

/**
  * @brief  Application entry point for the LCD 1602 counter demo.
  * @retval int does not return
  */
int main(void)
{
  xosc_set_clk_ref();
  i2c_release_reset();
  i2c_init();
  lcd_init();
  uart_puts("LCD 1602 driver initialized at I2C addr 0x27\r\n");
  lcd_set_cursor(0, 0);
  lcd_puts("Reverse Eng.");
  uint32_t count = 0;
  while (1) {
    char buf[17];
    _format_counter(count, buf);
    lcd_set_cursor(1, 0);
    lcd_puts(buf);
    uart_puts(buf);
    uart_puts("\r\n");
    count++;
    delay_ms(COUNT_DELAY_MS);
  }
}

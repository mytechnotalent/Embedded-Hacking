/**
 * @file rp2350_lcd1602.h
 * @brief LCD1602 driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * HD44780 16x2 LCD driver via PCF8574 I2C backpack. Uses I2C1
 * to communicate with the PCF8574 expander at address 0x27 in
 * 4-bit mode with backlight control.
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
#ifndef __RP2350_LCD1602_H
#define __RP2350_LCD1602_H

#include "rp2350.h"

/**
  * @brief  Initialize the LCD in 4-bit mode via the PCF8574 backpack.
  *
  *         Sets the I2C target address to LCD_I2C_ADDR, performs the
  *         HD44780 power-on reset sequence, and configures the display
  *         for 4-bit, 2-line, 5x8 font with backlight on.
  *
  * @retval None
  */
void lcd_init(void);

/**
  * @brief  Clear the LCD display and return cursor to home.
  * @retval None
  */
void lcd_clear(void);

/**
  * @brief  Set the cursor position on the display.
  * @param  line line number (0 or 1)
  * @param  position column number (0-15)
  * @retval None
  */
void lcd_set_cursor(uint8_t line, uint8_t position);

/**
  * @brief  Write a null-terminated string to the display.
  * @param  str pointer to the string to write (ASCII)
  * @retval None
  */
void lcd_puts(const char *str);

#endif /* __RP2350_LCD1602_H */

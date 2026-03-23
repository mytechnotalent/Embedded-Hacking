/**
 * @file lcd1602.h
 * @brief Header for PCF8574-backed HD44780 (16x2) LCD driver
 * @author Kevin Thomas
 * @date 2025
 *
 * MIT License
 *
 * Copyright (c) 2025 Kevin Thomas
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

#ifndef LCD1602_H
#define LCD1602_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize I2C bus and the LCD driver in one call
 *
 * Configures the I2C peripheral at the given baud rate, assigns GPIO
 * alternate functions for SDA and SCL with internal pull-ups, and then
 * performs the full HD44780 4-bit initialization sequence through the
 * PCF8574 backpack. This is a convenience wrapper around lcd_i2c_init().
 *
 * @param i2c_port       I2C port number (0 for i2c0, 1 for i2c1)
 * @param sda_pin        GPIO pin number for SDA
 * @param scl_pin        GPIO pin number for SCL
 * @param baud_hz        I2C clock rate in Hz (e.g. 100000)
 * @param pcf_addr       PCF8574 I2C address (commonly 0x27 or 0x3F)
 * @param nibble_shift   Bit shift applied to 4-bit nibbles (commonly 4 or 0)
 * @param backlight_mask PCF8574 bit mask that controls the backlight
 */
void lcd_init(uint8_t i2c_port, uint32_t sda_pin, uint32_t scl_pin,
              uint32_t baud_hz, uint8_t pcf_addr, int nibble_shift,
              uint8_t backlight_mask);

/**
 * @brief Initialize the LCD driver over I2C
 *
 * Configures the internal driver state and performs the HD44780 initialization
 * sequence. The driver does not configure I2C pins or call i2c_init; that
 * must be done by the caller prior to calling this function.
 *
 * @param i2c_port I2C port number (0 for i2c0, 1 for i2c1)
 * @param pcf_addr PCF8574 I2C address (commonly 0x27 or 0x3F)
 * @param nibble_shift Bit shift applied to 4-bit nibbles (commonly 4 or 0)
 * @param backlight_mask PCF8574 bit mask that controls the backlight
 */
void lcd_i2c_init(uint8_t i2c_port, uint8_t pcf_addr, int nibble_shift, uint8_t backlight_mask);

/**
 * @brief Clear the LCD display
 *
 * Clears the display and returns the cursor to the home position. This
 * call blocks for the duration required by the HD44780 controller.
 */
void lcd_clear(void);

/**
 * @brief Set the cursor position
 *
 * @param line Line number (0 or 1)
 * @param position Column (0..15)
 */
void lcd_set_cursor(int line, int position);

/**
 * @brief Write a null-terminated string to the display
 *
 * @param s The string to write (ASCII)
 */
void lcd_puts(const char *s);

#endif // LCD1602_H

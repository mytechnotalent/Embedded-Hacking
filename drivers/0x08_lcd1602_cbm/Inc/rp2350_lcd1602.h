/**
  ******************************************************************************
  * @file    rp2350_lcd1602.h
  * @author  Kevin Thomas
  * @brief   LCD1602 driver header for RP2350.
  *
  *          HD44780 16x2 LCD driver via PCF8574 I2C backpack. Uses I2C1
  *          to communicate with the PCF8574 expander at address 0x27 in
  *          4-bit mode with backlight control.
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

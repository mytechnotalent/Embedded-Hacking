/**
  ******************************************************************************
  * @file    rp2350_lcd1602.c
  * @author  Kevin Thomas
  * @brief   LCD1602 driver implementation for RP2350.
  *
  *          Drives a 16x2 HD44780 LCD through a PCF8574 I2C backpack
  *          in 4-bit mode. Each nibble is latched by pulsing the EN
  *          line via single-byte I2C writes to the PCF8574. All
  *          timing margins exceed HD44780 datasheet minimums.
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

#include "rp2350_lcd1602.h"
#include "rp2350_i2c.h"
#include "rp2350_delay.h"

/**
  * @brief  Pulse the EN line to latch a nibble into the HD44780.
  *
  *         Writes the data byte with EN set, waits 1 us, then writes
  *         with EN cleared and waits 50 us for the controller to process.
  *
  * @param  data PCF8574 output byte (RS, backlight, and nibble already set)
  * @retval None
  */
static void _lcd_pulse_enable(uint8_t data)
{
  i2c_write_byte(data | LCD_PIN_EN);
  delay_us(1);
  i2c_write_byte(data & ~LCD_PIN_EN);
  delay_us(50);
}

/**
  * @brief  Write one 4-bit nibble to the LCD controller.
  * @param  nibble lower 4 bits to send
  * @param  mode   0 for command, non-zero for character data
  * @retval None
  */
static void _lcd_write4(uint8_t nibble, uint8_t mode)
{
  uint8_t data = (nibble & 0x0FU) << LCD_NIBBLE_SHIFT;
  data |= mode ? LCD_PIN_RS : 0U;
  data |= LCD_BACKLIGHT;
  _lcd_pulse_enable(data);
}

/**
  * @brief  Send a full 8-bit value as two nibbles (high then low).
  * @param  value byte to send to the LCD
  * @param  mode  0 for command, non-zero for character data
  * @retval None
  */
static void _lcd_send(uint8_t value, uint8_t mode)
{
  _lcd_write4((value >> 4) & 0x0FU, mode);
  _lcd_write4(value & 0x0FU, mode);
}

/**
  * @brief  Execute the HD44780 4-bit mode power-on reset sequence.
  *
  *         Sends 0x03 three times with required inter-command delays,
  *         then sends 0x02 to switch from 8-bit to 4-bit interface.
  *
  * @retval None
  */
static void _lcd_hd44780_reset(void)
{
  _lcd_write4(0x03, 0);
  delay_ms(5);
  _lcd_write4(0x03, 0);
  delay_us(150);
  _lcd_write4(0x03, 0);
  delay_us(150);
  _lcd_write4(0x02, 0);
  delay_us(150);
}

/**
  * @brief  Send post-reset configuration commands to the HD44780.
  *
  *         Sets 4-bit mode with 2 display lines and 5x8 font, turns
  *         display on with cursor hidden, clears the screen, and
  *         selects left-to-right entry mode.
  *
  * @retval None
  */
static void _lcd_hd44780_configure(void)
{
  _lcd_send(LCD_CMD_FUNCTION_SET_4BIT, 0);
  _lcd_send(LCD_CMD_DISPLAY_ON, 0);
  _lcd_send(LCD_CMD_CLEAR, 0);
  delay_ms(2);
  _lcd_send(LCD_CMD_ENTRY_MODE, 0);
}

/**
  * @brief  Initialize the LCD in 4-bit mode via the PCF8574 backpack.
  *
  *         Sets the I2C target address to LCD_I2C_ADDR, performs the
  *         HD44780 power-on reset sequence, and configures the display
  *         for 4-bit, 2-line, 5x8 font with backlight on.
  *
  * @retval None
  */
void lcd_init(void)
{
  i2c_set_target(LCD_I2C_ADDR);
  _lcd_hd44780_reset();
  _lcd_hd44780_configure();
}

/**
  * @brief  Clear the LCD display and return cursor to home.
  * @retval None
  */
void lcd_clear(void)
{
  _lcd_send(LCD_CMD_CLEAR, 0);
  delay_ms(2);
}

/**
  * @brief  Set the cursor position on the display.
  * @param  line line number (0 or 1)
  * @param  position column number (0-15)
  * @retval None
  */
void lcd_set_cursor(uint8_t line, uint8_t position)
{
  uint8_t offset = (line == 0U) ? LCD_ROW0_OFFSET : LCD_ROW1_OFFSET;
  _lcd_send(LCD_CMD_SET_DDRAM | (position + offset), 0);
}

/**
  * @brief  Write a null-terminated string to the display.
  * @param  str pointer to the string to write (ASCII)
  * @retval None
  */
void lcd_puts(const char *str)
{
  while (*str)
    _lcd_send((uint8_t)*str++, 1);
}

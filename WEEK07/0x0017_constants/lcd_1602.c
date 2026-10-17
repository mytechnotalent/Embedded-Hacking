/**
 * @file lcd_1602.c
 * @brief Implementation of PCF8574-backed HD44780 (16x2) LCD driver
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

#include "lcd_1602.h"
#include <string.h>
#include <stdint.h>

/** @brief I2C instance pointer for the LCD */
static i2c_inst_t *lcd_i2c = NULL;
/** @brief I2C address of the PCF8574 backpack */
static uint8_t lcd_addr = 0x27;
/** @brief Bit shift for 4-bit nibble position */
static int lcd_nibble_shift = 4;
/** @brief PCF8574 bit mask controlling the backlight */
static uint8_t lcd_backlight_mask = 0x08;

/** @brief PCF8574 bit mask for Register Select */
#define PIN_RS 0x01
/** @brief PCF8574 bit mask for Read/Write */
#define PIN_RW 0x02
/** @brief PCF8574 bit mask for Enable */
#define PIN_EN 0x04

/**
 * @brief Write one raw byte to the PCF8574 expander over I2C
 *
 * @param data Output byte to send to the expander
 */
static void pcf_write_byte(uint8_t data) {
    if (!lcd_i2c) return;
    i2c_write_blocking(lcd_i2c, lcd_addr, &data, 1, false);
}

/**
 * @brief Toggle EN to latch a nibble into the LCD controller
 *
 * @param data Current control/data bus byte (with RS and backlight already set)
 */
static void pcf_pulse_enable(uint8_t data) {
    pcf_write_byte(data | PIN_EN);
    sleep_us(1);
    pcf_write_byte(data & ~PIN_EN);
    sleep_us(50);
}

/**
 * @brief Write one 4-bit nibble to the LCD
 *
 * @param nibble Lower 4 bits to write
 * @param mode   0 for command, non-zero for character data
 */
static void lcd_write4(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0x0F) << lcd_nibble_shift;
    data |= mode ? PIN_RS : 0;
    data |= lcd_backlight_mask;
    pcf_pulse_enable(data);
}

/**
 * @brief Send one full 8-bit command/data value as two nibbles
 *
 * @param value Byte to send to the LCD
 * @param mode  0 for command, non-zero for character data
 */
static void lcd_send(uint8_t value, uint8_t mode) {
    lcd_write4((value >> 4) & 0x0F, mode);
    lcd_write4(value & 0x0F, mode);
}

/**
 * @brief Store LCD driver configuration in module-level state
 *
 * @param i2c            Pointer to the I2C instance
 * @param pcf_addr       7-bit PCF8574 address
 * @param nibble_shift   Bit shift for 4-bit nibbles
 * @param backlight_mask Backlight control bit mask
 */
static void lcd_store_config(i2c_inst_t *i2c, uint8_t pcf_addr,
                             int nibble_shift, uint8_t backlight_mask) {
    lcd_i2c = i2c;
    lcd_addr = pcf_addr;
    lcd_nibble_shift = nibble_shift;
    lcd_backlight_mask = backlight_mask;
}

/**
 * @brief Execute the HD44780 4-bit mode power-on reset sequence
 */
static void lcd_hd44780_reset(void) {
    lcd_write4(0x03, 0);
    sleep_ms(5);
    lcd_write4(0x03, 0);
    sleep_us(150);
    lcd_write4(0x03, 0);
    sleep_us(150);
    lcd_write4(0x02, 0);
    sleep_us(150);
}

/**
 * @brief Send post-reset configuration commands to the HD44780
 *
 * Sets 4-bit mode with 2 display lines, turns the display on with
 * cursor hidden, clears the screen, and selects left-to-right entry mode.
 */
static void lcd_hd44780_configure(void) {
    lcd_send(0x28, 0);
    lcd_send(0x0C, 0);
    lcd_send(0x01, 0);
    sleep_ms(2);
    lcd_send(0x06, 0);
}

void lcd_i2c_init(i2c_inst_t *i2c, uint8_t pcf_addr, int nibble_shift, uint8_t backlight_mask) {
    lcd_store_config(i2c, pcf_addr, nibble_shift, backlight_mask);
    lcd_hd44780_reset();
    lcd_hd44780_configure();
}

void lcd_clear(void) {
    lcd_send(0x01, 0);
    sleep_ms(2);
}

void lcd_set_cursor(int line, int position) {
    const uint8_t row_offsets[] = {0x00, 0x40};
    if (line > 1) line = 1;
    lcd_send(0x80 | (position + row_offsets[line]), 0);
}

void lcd_puts(const char *s) {
    while (*s)
        lcd_send((uint8_t)*s++, 1);
}

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

static i2c_inst_t *lcd_i2c = NULL;
static uint8_t lcd_addr = 0x27;
static int lcd_nibble_shift = 4;
static uint8_t lcd_backlight_mask = 0x08;

/* PCF8574 -> LCD control pins */
#define PIN_RS 0x01
#define PIN_RW 0x02
#define PIN_EN 0x04

static void pcf_write_byte(uint8_t data) {
    if (!lcd_i2c) return;
    i2c_write_blocking(lcd_i2c, lcd_addr, &data, 1, false);
}

static void pcf_pulse_enable(uint8_t data) {
    pcf_write_byte(data | PIN_EN);
    sleep_us(1);
    pcf_write_byte(data & ~PIN_EN);
    sleep_us(50);
}

static void lcd_write4(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0x0F) << lcd_nibble_shift;
    data |= mode ? PIN_RS : 0;
    data |= lcd_backlight_mask;
    pcf_pulse_enable(data);
}

static void lcd_send(uint8_t value, uint8_t mode) {
    lcd_write4((value >> 4) & 0x0F, mode);
    lcd_write4(value & 0x0F, mode);
}

void lcd_i2c_init(i2c_inst_t *i2c, uint8_t pcf_addr, int nibble_shift, uint8_t backlight_mask) {
    lcd_i2c = i2c;
    lcd_addr = pcf_addr;
    lcd_nibble_shift = nibble_shift;
    lcd_backlight_mask = backlight_mask;

    // Follow init sequence for HD44780 in 4-bit mode
    lcd_write4(0x03, 0);
    sleep_ms(5);
    lcd_write4(0x03, 0);
    sleep_us(150);
    lcd_write4(0x03, 0);
    sleep_us(150);
    lcd_write4(0x02, 0);
    sleep_us(150);

    // Function set: 4-bit, 2 lines
    lcd_send(0x28, 0);

    // Display on, cursor off
    lcd_send(0x0C, 0);

    // Clear
    lcd_send(0x01, 0);
    sleep_ms(2);

    // Entry mode set
    lcd_send(0x06, 0);
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
    while (*s) {
        lcd_send((uint8_t)*s++, 1);
    }
}

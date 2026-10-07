// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    lcd.c
// Desc:    Implements I2C HD44780 16x2 LCD display for live telemetry coordinates.
// Created: 2026

#include "lcd.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define PIN_RS     0x01
#define PIN_EN     0x04
#define BACKLIGHT  0x08

static uint8_t lcd_addr = 0x27;
static bool lcd_ready = false;

static void pcf_write(uint8_t d)
{
    if (lcd_ready)
        i2c_write_blocking(LCD_I2C_INST, lcd_addr, &d, 1, false);
}

static void pcf_pulse(uint8_t d)
{
    pcf_write(d | PIN_EN);
    sleep_us(1);
    pcf_write(d & ~PIN_EN);
    sleep_us(50);
}

static void lcd_write4(uint8_t n, uint8_t mode)
{
    uint8_t d = (n & 0x0F) << 4;
    d |= mode ? PIN_RS : 0;
    d |= BACKLIGHT;
    pcf_pulse(d);
}

static void lcd_send(uint8_t v, uint8_t mode)
{
    lcd_write4((v >> 4) & 0x0F, mode);
    lcd_write4(v & 0x0F, mode);
}

static void lcd_clear(void)
{
    lcd_send(0x01, 0);
    sleep_ms(2);
}

static void lcd_set_cursor(int row, int col)
{
    uint8_t offset = (row == 0) ? 0x00 : 0x40;
    lcd_send(0x80 | (col + offset), 0);
}

static void lcd_puts(const char *s)
{
    while (*s)
        lcd_send((uint8_t)*s++, 1);
}

static void lcd_reset_seq(void)
{
    lcd_write4(0x03, 0); sleep_ms(5);
    lcd_write4(0x03, 0); sleep_us(150);
    lcd_write4(0x03, 0); sleep_us(150);
    lcd_write4(0x02, 0); sleep_us(150);
}

static void lcd_cfg_seq(void)
{
    lcd_send(0x28, 0);
    lcd_send(0x0C, 0);
    lcd_clear();
    lcd_send(0x06, 0);
}

static bool detect_lcd(void)
{
    uint8_t rx;
    if (i2c_read_blocking(LCD_I2C_INST, 0x27, &rx, 1, false) >= 0)
        return (lcd_addr = 0x27, true);
    if (i2c_read_blocking(LCD_I2C_INST, 0x3F, &rx, 1, false) >= 0)
        return (lcd_addr = 0x3F, true);
    return false;
}

void init_lcd(void)
{
    i2c_init(LCD_I2C_INST, LCD_BAUD);
    gpio_set_function(LCD_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(LCD_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(LCD_SDA_PIN); gpio_pull_up(LCD_SCL_PIN);
    if (!(lcd_ready = detect_lcd())) return;
    lcd_reset_seq();
    lcd_cfg_seq();
}

void lcd_show_coords(double lat, double lon)
{
    if (!lcd_ready && !(lcd_ready = detect_lcd())) return;
    char r1[17], r2[17];
    snprintf(r1, sizeof(r1), "LAT: %9.6f %c", fabs(lat), (lat >= 0.0) ? 'N' : 'S');
    snprintf(r2, sizeof(r2), "LON: %9.6f %c", fabs(lon), (lon >= 0.0) ? 'E' : 'W');
    lcd_set_cursor(0, 0); lcd_puts(r1);
    lcd_set_cursor(1, 0); lcd_puts(r2);
}

void lcd_show_gnss(int sats, int cno)
{
    if (!lcd_ready && !(lcd_ready = detect_lcd())) return;
    char r1[17], r2[17];
    snprintf(r1, sizeof(r1), "SAT:%2d CNO:%2d   ", sats, cno);
    snprintf(r2, sizeof(r2), "%-16s", (sats > 0) ? "ACQUIRING..." : "NO SIGNAL");
    lcd_set_cursor(0, 0); lcd_puts(r1);
    lcd_set_cursor(1, 0); lcd_puts(r2);
}

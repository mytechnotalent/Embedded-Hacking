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
// File:    lcd.h
// Desc:    Declares I2C HD44780 (16x2) LCD interface for telemetry coordinates.
// Created: 2026

#ifndef LCD_H
#define LCD_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

#define LCD_I2C_INST  i2c1
#define LCD_SDA_PIN   2
#define LCD_SCL_PIN   3
#define LCD_BAUD      100000

/**
 * @brief Initialize I2C0 peripheral and detect/configure 1602 LCD backpack.
 *
 * @param None.
 * @return None.
 */
void init_lcd(void);

/**
 * @brief Render current latitude and longitude on the 16x2 character display.
 *
 * Row 0: LAT: dd.dddddd N
 * Row 1: LON: dd.dddddd W
 *
 * @param lat Current latitude in decimal degrees.
 * @param lon Current longitude in decimal degrees.
 * @return None.
 */
void lcd_show_coords(double lat, double lon);

/**
 * @brief Render GNSS acquisition telemetry (satellites and C/N0) on the LCD.
 *
 * Row 0: SAT: nn CNO: nn
 * Row 1: ACQUIRING... when satellites are in view, else NO SIGNAL
 *
 * @param sats Number of satellites currently in view.
 * @param cno Best carrier-to-noise ratio in dBHz.
 * @return None.
 */
void lcd_show_gnss(int sats, int cno);

#endif // LCD_H

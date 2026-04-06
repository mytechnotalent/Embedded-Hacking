/**
 * @file 0x08_lcd1602.c
 * @brief HD44780 16x2 LCD (PCF8574 I2C backpack) driver for the Raspberry Pi Pico 2
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
 *
 * -----------------------------------------------------------------------------
 *
 * This driver demonstrates how to drive a 16x2 HD44780 LCD connected via a
 * PCF8574 I2C backpack using the lcd1602.c/lcd1602.h driver. Line 0 shows
 * a static title and line 1 displays a live up-counter that increments every
 * second. The counter value is also printed over UART for debugging.
 *
 * Wiring:
 *   GPIO2 (SDA) -> PCF8574 backpack SDA  (4.7 kohm pull-up to 3.3 V)
 *   GPIO3 (SCL) -> PCF8574 backpack SCL  (4.7 kohm pull-up to 3.3 V)
 *   3.3V or 5V  -> PCF8574 backpack VCC
 *   GND         -> PCF8574 backpack GND
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "lcd1602.h"

/** @brief I2C port number (0 or 1) */
#define I2C_PORT      1
/** @brief GPIO pin for I2C SDA */
#define I2C_SDA_PIN   2
/** @brief GPIO pin for I2C SCL */
#define I2C_SCL_PIN   3
/** @brief I2C bus clock rate in Hz */
#define I2C_BAUD_HZ   100000
/** @brief I2C address of the PCF8574 LCD backpack */
#define LCD_I2C_ADDR  0x27

/**
 * @brief Initialize the LCD, display the title, and log over UART
 */
static void _setup_display(void) {
    lcd_init(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_BAUD_HZ, LCD_I2C_ADDR, 4, 0x08);
    lcd_set_cursor(0, 0);
    lcd_puts("Reverse Eng.");
    printf("LCD 1602 driver initialized at I2C addr 0x%02X\r\n", LCD_I2C_ADDR);
}

/**
 * @brief Format and display the next counter value on LCD line 1
 *
 * @param count Pointer to the running counter (post-incremented)
 */
static void _update_counter(uint32_t *count) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Count: %6lu", (*count)++);
    lcd_set_cursor(1, 0);
    lcd_puts(buf);
    printf("%s\r\n", buf);
    sleep_ms(1000);
}

/**
 * @brief Application entry point for the LCD 1602 counter demo
 *
 * Initializes the LCD over I2C with a static title on line 0 and
 * continuously increments a counter on line 1 every second.
 *
 * @return int Does not return
 */
int main(void) {
    stdio_init_all();
    _setup_display();
    uint32_t count = 0;
    while (true)
        _update_counter(&count);
}

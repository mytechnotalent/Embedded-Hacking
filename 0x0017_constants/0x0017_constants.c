/**
 * @file 0x0017_constants.c
 * @brief Constants: demonstrate #define and const with LCD1602 I2C display
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
 * Demonstrates #define macro constants and const-qualified variables.
 * Initializes an LCD1602 display over I2C and prints constant values
 * over UART in an infinite loop.
 *
 * Wiring:
 *   GPIO2 (SDA) -> LCD1602 PCF8574 SDA
 *   GPIO3 (SCL) -> LCD1602 PCF8574 SCL
 *   3V3         -> LCD1602 VCC
 *   GND         -> LCD1602 GND
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_1602.h"

/** @brief Macro constant for favorite number */
#define FAV_NUM     42
/** @brief I2C peripheral instance */
#define I2C_PORT    i2c1
/** @brief GPIO pin for I2C SDA */
#define I2C_SDA_PIN 2
/** @brief GPIO pin for I2C SCL */
#define I2C_SCL_PIN 3

/** @brief Const-qualified favorite number */
const int OTHER_FAV_NUM = 1337;

/**
 * @brief Initialize I2C bus and LCD1602 display
 *
 * @details Configures I2C1 at 100kHz on GPIO2/3, initializes the
 *          LCD via PCF8574 at address 0x27, and writes two lines.
 *
 * @retval None
 */
static void init_i2c_and_lcd(void) {
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    lcd_i2c_init(I2C_PORT, 0x27, 4, 0x08);
}

/**
 * @brief Write greeting text to the LCD display
 *
 * @details Sets cursor to line 0 and writes "Reverse", then sets
 *          cursor to line 1 and writes "Engineering".
 *
 * @retval None
 */
static void write_lcd_greeting(void) {
    lcd_set_cursor(0, 0);
    lcd_puts("Reverse");
    lcd_set_cursor(1, 0);
    lcd_puts("Engineering");
}

int main(void) {
    stdio_init_all();
    init_i2c_and_lcd();
    write_lcd_greeting();
    while (true) {
        printf("FAV_NUM: %d\r\n", FAV_NUM);
        printf("OTHER_FAV_NUM: %d\r\n", OTHER_FAV_NUM);
    }
}

/**
 * @file lcd1602.c
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

#include "lcd1602.h"
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/**
 * @brief Map an I2C port number to its hardware instance pointer
 *
 * @param port I2C port number (0 for i2c0, 1 for i2c1)
 * @return i2c_inst_t* Pointer to the corresponding I2C hardware instance
 */
static i2c_inst_t *_get_i2c_inst(uint8_t port) {
    return port == 0 ? i2c0 : i2c1;
}


static i2c_inst_t *lcd_i2c = NULL;
static uint8_t lcd_addr = 0x27;
static int lcd_nibble_shift = 4;
static uint8_t lcd_backlight_mask = 0x08;

/* PCF8574 -> LCD control pins */
#define PIN_RS 0x01
#define PIN_RW 0x02
#define PIN_EN 0x04


/**
 * @brief Write one raw byte to the PCF8574 expander over I2C
 *
 * @param data Output byte to send to the expander
 */
static void _pcf_write_byte(uint8_t data) {
    if (!lcd_i2c) return;
    i2c_write_blocking(lcd_i2c, lcd_addr, &data, 1, false);
}


/**
 * @brief Toggle EN to latch a nibble into the LCD controller
 *
 * @param data Current control/data bus byte (with RS and backlight already set)
 */
static void _pcf_pulse_enable(uint8_t data) {
    _pcf_write_byte(data | PIN_EN);
    sleep_us(1);
    _pcf_write_byte(data & ~PIN_EN);
    sleep_us(50);
}


/**
 * @brief Write one 4-bit nibble to the LCD
 *
 * @param nibble Lower 4 bits to write
 * @param mode   0 for command, non-zero for character data
 */
static void _lcd_write4(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0x0F) << lcd_nibble_shift;
    data |= mode ? PIN_RS : 0;
    data |= lcd_backlight_mask;
    _pcf_pulse_enable(data);
}


/**
 * @brief Send one full 8-bit command/data value as two nibbles
 *
 * @param value Byte to send to the LCD
 * @param mode  0 for command, non-zero for character data
 */
static void _lcd_send(uint8_t value, uint8_t mode) {
    _lcd_write4((value >> 4) & 0x0F, mode);
    _lcd_write4(value & 0x0F, mode);
}


/**
 * @brief Store LCD driver configuration in module-level state
 *
 * @param i2c_port       I2C port number (0 or 1)
 * @param pcf_addr       7-bit PCF8574 address
 * @param nibble_shift   Bit shift for 4-bit nibbles
 * @param backlight_mask Backlight control bit mask
 */
static void _lcd_store_config(uint8_t i2c_port, uint8_t pcf_addr,
                              int nibble_shift, uint8_t backlight_mask) {
    lcd_i2c = _get_i2c_inst(i2c_port);
    lcd_addr = pcf_addr;
    lcd_nibble_shift = nibble_shift;
    lcd_backlight_mask = backlight_mask;
}


/**
 * @brief Execute the HD44780 4-bit mode power-on reset sequence
 */
static void _lcd_hd44780_reset(void) {
    _lcd_write4(0x03, 0);
    sleep_ms(5);
    _lcd_write4(0x03, 0);
    sleep_us(150);
    _lcd_write4(0x03, 0);
    sleep_us(150);
    _lcd_write4(0x02, 0);
    sleep_us(150);
}


/**
 * @brief Send post-reset configuration commands to the HD44780
 *
 * Sets 4-bit mode with 2 display lines, turns the display on with
 * cursor hidden, clears the screen, and selects left-to-right entry mode.
 */
static void _lcd_hd44780_configure(void) {
    _lcd_send(0x28, 0);
    _lcd_send(0x0C, 0);
    _lcd_send(0x01, 0);
    sleep_ms(2);
    _lcd_send(0x06, 0);
}


void lcd_i2c_init(uint8_t i2c_port, uint8_t pcf_addr, int nibble_shift,
                  uint8_t backlight_mask) {
    _lcd_store_config(i2c_port, pcf_addr, nibble_shift, backlight_mask);
    _lcd_hd44780_reset();
    _lcd_hd44780_configure();
}


void lcd_init(uint8_t i2c_port, uint32_t sda_pin, uint32_t scl_pin,
              uint32_t baud_hz, uint8_t pcf_addr, int nibble_shift,
              uint8_t backlight_mask) {
    i2c_inst_t *i2c = _get_i2c_inst(i2c_port);
    i2c_init(i2c, baud_hz);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
    lcd_i2c_init(i2c_port, pcf_addr, nibble_shift, backlight_mask);
}


void lcd_clear(void) {
    _lcd_send(0x01, 0);
    sleep_ms(2);
}


void lcd_set_cursor(int line, int position) {
    const uint8_t row_offsets[] = {0x00, 0x40};
    if (line > 1) line = 1;
    _lcd_send(0x80 | (position + row_offsets[line]), 0);
}


void lcd_puts(const char *s) {
    while (*s)
        _lcd_send((uint8_t)*s++, 1);
}

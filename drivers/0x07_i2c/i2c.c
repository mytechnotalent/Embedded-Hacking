/**
 * @file i2c.c
 * @brief Implementation of the I2C bus driver
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

#include "i2c.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

/**
 * @brief Map an I2C port number to its hardware instance pointer
 *
 * @param port I2C port number (0 for i2c0, 1 for i2c1)
 * @return i2c_inst_t* Pointer to the corresponding I2C hardware instance
 */
static i2c_inst_t *_get_i2c_inst(uint8_t port) {
    return port == 0 ? i2c0 : i2c1;
}

void i2c_driver_init(uint8_t port, uint32_t sda_pin, uint32_t scl_pin,
                     uint32_t baud_hz) {
    i2c_inst_t *inst = _get_i2c_inst(port);
    i2c_init(inst, baud_hz);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

bool i2c_driver_probe(uint8_t port, uint8_t addr) {
    i2c_inst_t *inst = _get_i2c_inst(port);
    uint8_t dummy;
    return i2c_read_blocking(inst, addr, &dummy, 1, false) >= 0;
}

/**
 * @brief Print the I2C scan table header over UART
 */
static void _print_scan_header(void) {
    printf("\r\nI2C bus scan:\r\n");
    printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");
}

/**
 * @brief Print one cell of the scan table for a given address
 *
 * Prints the row label when the address is at a 16-byte boundary, then
 * prints the address if a device responds, dashes if not, or blank if
 * the address is in the reserved range. Ends the row at every 16th address.
 *
 * @param port I2C port number (0 for i2c0, 1 for i2c1)
 * @param addr 7-bit I2C address being probed
 */
static void _print_scan_entry(uint8_t port, uint8_t addr) {
    if (addr % 16 == 0) printf("%02X: ", addr);
    if (addr < 0x08 || addr > 0x77) printf("   ");
    else if (i2c_driver_probe(port, addr)) printf("%02X ", addr);
    else printf("-- ");
    if (addr % 16 == 15) printf("\r\n");
}

void i2c_driver_scan(uint8_t port) {
    _print_scan_header();
    for (uint8_t addr = 0; addr < 128; addr++)
        _print_scan_entry(port, addr);
}

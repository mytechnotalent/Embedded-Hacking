/**
 * @file i2c.h
 * @brief Header for I2C bus driver (init + 7-bit address scanner)
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

#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize an I2C peripheral at the requested baud rate
 *
 * Calls i2c_init() for the given instance, assigns the GPIO alternate
 * functions for SDA and SCL, and enables the internal pull-ups on both
 * pins so that short buses operate without external resistors.
 *
 * @param port    I2C port number (0 for i2c0, 1 for i2c1)
 * @param sda_pin GPIO pin number for SDA
 * @param scl_pin GPIO pin number for SCL
 * @param baud_hz Bus clock frequency in Hz (e.g. 100000 for standard mode)
 */
void i2c_driver_init(uint8_t port, uint32_t sda_pin, uint32_t scl_pin,
                     uint32_t baud_hz);

/**
 * @brief Probe a 7-bit I2C address and return whether a device responds
 *
 * Attempts a 1-byte read to the target address. A non-negative return from
 * i2c_read_blocking() means the device sent an ACK. A negative return means
 * the address is unoccupied.
 *
 * @param port I2C port number (0 for i2c0, 1 for i2c1)
 * @param addr 7-bit I2C address to probe (0x00 - 0x7F)
 * @return bool true if a device acknowledged, false otherwise
 */
bool i2c_driver_probe(uint8_t port, uint8_t addr);

/**
 * @brief Scan all valid 7-bit addresses and print a formatted table over UART
 *
 * Iterates addresses 0x08 through 0x77, probes each one via i2c_driver_probe(),
 * and prints a 16-column hex grid showing discovered device addresses.
 * The reserved ranges (0x00-0x07 and 0x78-0x7F) are shown as blank.
 *
 * @param port I2C port number (0 for i2c0, 1 for i2c1)
 */
void i2c_driver_scan(uint8_t port);

#endif // I2C_H

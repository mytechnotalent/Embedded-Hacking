/**
 * @file spi.c
 * @brief Implementation of SPI bus driver (master mode)
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

#include "spi.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

/**
 * @brief Map an SPI port number to its hardware instance pointer
 *
 * @param port SPI port number (0 for spi0, 1 for spi1)
 * @return spi_inst_t* Pointer to the corresponding SPI hardware instance
 */
static spi_inst_t *_get_spi_inst(uint8_t port) {
    return port == 0 ? spi0 : spi1;
}

/**
 * @brief Assign GPIO alternate functions for the SPI data and clock pins
 *
 * @param mosi GPIO pin for MOSI
 * @param miso GPIO pin for MISO
 * @param sck  GPIO pin for SCK
 */
static void _setup_spi_pins(uint32_t mosi, uint32_t miso, uint32_t sck) {
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(miso, GPIO_FUNC_SPI);
    gpio_set_function(sck, GPIO_FUNC_SPI);
}

void spi_driver_init(uint8_t port, uint32_t mosi, uint32_t miso,
                     uint32_t sck, uint32_t cs, uint32_t baud_hz) {
    spi_inst_t *spi = _get_spi_inst(port);
    spi_init(spi, baud_hz);
    _setup_spi_pins(mosi, miso, sck);
    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);
}

void spi_driver_cs_select(uint32_t cs) {
    gpio_put(cs, 0);
}

void spi_driver_cs_deselect(uint32_t cs) {
    gpio_put(cs, 1);
}

void spi_driver_transfer(uint8_t port, const uint8_t *tx, uint8_t *rx,
                         uint32_t len) {
    spi_inst_t *spi = _get_spi_inst(port);
    spi_write_read_blocking(spi, tx, rx, (size_t)len);
}

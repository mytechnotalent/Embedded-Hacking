/**
 * @file 0x0b_spi.c
 * @brief SPI loopback demo using spi.c/spi.h driver
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
 * Demonstrates SPI in master mode using the spi driver (spi.h / spi.c).
 * A loopback wiring (MOSI -> MISO) is used to verify full-duplex transfer.
 * Transmitted and received data is printed over UART every second.
 *
 * Wiring (loopback test):
 *   GPIO19 (MOSI) -> GPIO16 (MISO)
 *   GPIO18 (SCK)  -> logic analyzer or slave SCK
 *   GPIO17 (CS)   -> slave CS (active-low)
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "spi.h"

#define SPI_PORT     0
#define SPI_BAUD_HZ  (1000 * 1000)
#define PIN_MISO     16
#define PIN_CS       17
#define PIN_SCK      18
#define PIN_MOSI     19

int main(void) {
    stdio_init_all();

    spi_driver_init(SPI_PORT, PIN_MOSI, PIN_MISO, PIN_SCK, PIN_CS, SPI_BAUD_HZ);

    const uint8_t tx_buf[] = "SPI loopback OK";
    uint8_t rx_buf[sizeof(tx_buf)] = {0};

    while (true) {
        spi_driver_cs_select(PIN_CS);
        spi_driver_transfer(SPI_PORT, tx_buf, rx_buf, sizeof(tx_buf));
        spi_driver_cs_deselect(PIN_CS);

        printf("TX: %s\r\n", tx_buf);
        printf("RX: %s\r\n\r\n", rx_buf);

        memset(rx_buf, 0, sizeof(rx_buf));
        sleep_ms(1000);
    }
}

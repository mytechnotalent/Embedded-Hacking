/**
 * @file 0x09_dht11.c
 * @brief DHT11 temperature and humidity sensor driver for the Raspberry Pi Pico 2
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
 * This driver demonstrates how to read temperature and humidity from a DHT11
 * sensor using the dht11.c/dht11.h driver. The sensor is polled every 2
 * seconds (the minimum safe interval for the DHT11) and the results are
 * printed over UART. A failed read is reported so wiring issues are visible.
 *
 * Wiring:
 *   GPIO4 -> DHT11 DATA pin  (10 kohm pull-up to 3.3 V recommended)
 *   3.3V  -> DHT11 VCC
 *   GND   -> DHT11 GND
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "dht11.h"

/** @brief GPIO pin connected to the DHT11 data line */
#define DHT11_GPIO 4

/**
 * @brief Read the DHT11 sensor and print the result over UART
 *
 * Attempts a single read. On success prints humidity and temperature;
 * on failure prints a wiring-check message. Waits 2 s before returning
 * to respect the DHT11 minimum polling interval.
 */
static void _print_reading(void) {
    float humidity = 0.0f;
    float temperature = 0.0f;
    if (dht11_read(&humidity, &temperature))
        printf("Humidity: %.1f%%  Temperature: %.1f C\r\n", humidity, temperature);
    else
        printf("DHT11 read failed - check wiring on GPIO %d\r\n", DHT11_GPIO);
    sleep_ms(2000);
}

int main(void) {
    stdio_init_all();
    dht11_init(DHT11_GPIO);
    printf("DHT11 driver initialized on GPIO %d\r\n", DHT11_GPIO);
    while (true)
        _print_reading();
}

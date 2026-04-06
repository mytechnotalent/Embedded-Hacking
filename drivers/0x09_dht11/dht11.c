/**
 * @file dht11.c
 * @brief Implementation of DHT11 temperature and humidity sensor driver
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

#include "dht11.h"
#include "hardware/gpio.h"
#include "pico/time.h"

/** @brief GPIO pin connected to the DHT11 sensor */
static uint dht_pin;

/**
 * @brief Send the DHT11 start signal on the data pin
 *
 * Drives the pin LOW for 18 ms then HIGH for 40 us before switching
 * the pin to input mode to listen for the sensor response.
 */
static void _send_start_signal(void) {
    gpio_set_dir(dht_pin, GPIO_OUT);
    gpio_put(dht_pin, 0);
    sleep_ms(18);
    gpio_put(dht_pin, 1);
    sleep_us(40);
    gpio_set_dir(dht_pin, GPIO_IN);
}

/**
 * @brief Wait for the pin to leave a given logic level
 *
 * Spins until the pin no longer reads the specified level, returning
 * false if a timeout of 10 000 iterations is exceeded.
 *
 * @param level Logic level to wait through (0 or 1)
 * @return bool true once the level changed, false on timeout
 */
static bool _wait_for_level(int level) {
    uint32_t timeout = 10000;
    while (gpio_get(dht_pin) == level)
        if (--timeout == 0) return false;
    return true;
}

/**
 * @brief Wait for the DHT11 response after the start signal
 *
 * The sensor pulls LOW then HIGH then LOW again; each transition
 * is awaited with a timeout.
 *
 * @return bool true if the full response was received, false on timeout
 */
static bool _wait_response(void) {
    if (!_wait_for_level(1)) return false;
    if (!_wait_for_level(0)) return false;
    if (!_wait_for_level(1)) return false;
    return true;
}

/**
 * @brief Read a single bit from the DHT11 data stream
 *
 * Waits for the low-period to end, measures the high-period duration,
 * and shifts the result into the appropriate byte of the data array.
 *
 * @param data 5-byte array accumulating the received bits
 * @param i    Bit index (0-39)
 * @return bool true on success, false on timeout
 */
static bool _read_bit(uint8_t *data, int i) {
    if (!_wait_for_level(0)) return false;
    uint32_t start = time_us_32();
    if (!_wait_for_level(1)) return false;
    uint32_t duration = time_us_32() - start;
    data[i / 8] <<= 1;
    if (duration > 40) data[i / 8] |= 1;
    return true;
}

/**
 * @brief Read all 40 data bits from the DHT11
 *
 * @param data 5-byte array filled with the received data
 * @return bool true if all 40 bits were read, false on timeout
 */
static bool _read_40_bits(uint8_t *data) {
    for (int i = 0; i < 40; i++)
        if (!_read_bit(data, i)) return false;
    return true;
}

/**
 * @brief Verify the DHT11 checksum byte
 *
 * @param data 5-byte received data (bytes 0-3 plus checksum in byte 4)
 * @return bool true if the checksum matches, false otherwise
 */
static bool _validate_checksum(const uint8_t *data) {
    return data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF);
}

/**
 * @brief Initialize the DHT11 driver
 *
 * Configures the GPIO pin for the DHT11 sensor. This must be called before
 * using dht11_read().
 *
 * @param pin GPIO pin number connected to DHT11 signal
 */
void dht11_init(uint8_t pin) {
    dht_pin = pin;
    gpio_init(pin);
    gpio_pull_up(pin);
}

/**
 * @brief Read temperature and humidity from DHT11 sensor
 *
 * Performs the DHT11 communication protocol to read sensor data.
 *
 * @param humidity    Pointer to store humidity value (0-100%)
 * @param temperature Pointer to store temperature value in Celsius
 * @return true if read successful, false on error or timeout
 */
bool dht11_read(float *humidity, float *temperature) {
    uint8_t data[5] = {0};
    _send_start_signal();
    if (!_wait_response()) return false;
    if (!_read_40_bits(data)) return false;
    if (!_validate_checksum(data)) return false;
    *humidity = data[0] + data[1] * 0.1f;
    *temperature = data[2] + data[3] * 0.1f;
    return true;
}

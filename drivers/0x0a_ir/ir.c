/**
 * @file ir.c
 * @brief Implementation of NEC IR receiver (decoder)
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

#include "ir.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"

static unsigned int ir_pin = 0;


/**
 * @brief Wait for a GPIO pin to reach a given logic level
 *
 * Spins until the pin matches the requested level or a microsecond timeout
 * is exceeded. Returns the elapsed time in microseconds, or -1 on timeout.
 *
 * @param gpio       GPIO pin to monitor
 * @param level      Desired logic level (true = HIGH, false = LOW)
 * @param timeout_us Maximum wait in microseconds
 * @return int64_t   Elapsed microseconds, or -1 on timeout
 */
static int64_t _wait_for_level(unsigned int gpio, bool level, uint32_t timeout_us) {
    absolute_time_t start = get_absolute_time();
    while (gpio_get(gpio) != level) {
        if (absolute_time_diff_us(start, get_absolute_time()) > (int64_t)timeout_us)
            return -1;
    }
    return absolute_time_diff_us(start, get_absolute_time());
}


/**
 * @brief Wait for the NEC 9 ms leader pulse and 4.5 ms space
 *
 * @return bool true if a valid leader was detected, false on timeout
 */
static bool _wait_leader(void) {
    if (_wait_for_level(ir_pin, 0, 150000) < 0) return false;
    int64_t t = _wait_for_level(ir_pin, 1, 12000);
    if (t < 8000 || t > 10000) return false;
    t = _wait_for_level(ir_pin, 0, 7000);
    if (t < 3500 || t > 5000) return false;
    return true;
}


/**
 * @brief Read a single NEC-encoded bit from the IR receiver
 *
 * Measures the mark/space timing and shifts the result into the
 * appropriate byte of the data array.
 *
 * @param data 4-byte array accumulating received bits
 * @param i    Bit index (0-31)
 * @return bool true on success, false on timeout or protocol error
 */
static bool _read_nec_bit(uint8_t *data, int i) {
    if (_wait_for_level(ir_pin, 1, 1000) < 0) return false;
    int64_t t = _wait_for_level(ir_pin, 0, 2500);
    if (t < 200) return false;
    int byte_idx = i / 8;
    int bit_idx = i % 8;
    if (t > 1200) data[byte_idx] |= (1 << bit_idx);
    return true;
}


/**
 * @brief Read all 32 data bits of an NEC frame
 *
 * @param data 4-byte array filled with the received address and command
 * @return bool true if all 32 bits were read, false on timeout
 */
static bool _read_32_bits(uint8_t *data) {
    for (int i = 0; i < 32; ++i)
        if (!_read_nec_bit(data, i)) return false;
    return true;
}


/**
 * @brief Validate an NEC frame and extract the command byte
 *
 * Checks that the address and command pairs are bitwise-inverted.
 *
 * @param data 4-byte NEC frame (addr, ~addr, cmd, ~cmd)
 * @return int Command byte (0-255) on success, -1 on validation failure
 */
static int _validate_nec_frame(const uint8_t *data) {
    if ((uint8_t)(data[0] + data[1]) == 0xFF && (uint8_t)(data[2] + data[3]) == 0xFF)
        return data[2];
    return -1;
}


void ir_init(uint8_t pin) {
    ir_pin = pin;
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}


int ir_getkey(void) {
    if (!_wait_leader()) return -1;
    uint8_t data[4] = {0, 0, 0, 0};
    if (!_read_32_bits(data)) return -1;
    return _validate_nec_frame(data);
}

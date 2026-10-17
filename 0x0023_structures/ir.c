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

// Wait for 'gpio' to reach 'level' or timeout (microseconds). Return elapsed us or -1.
static int64_t wait_for_level(unsigned int gpio, bool level, uint32_t timeout_us) {
    absolute_time_t start = get_absolute_time();
    while (gpio_get(gpio) != level) {
        if (absolute_time_diff_us(start, get_absolute_time()) > (int64_t)timeout_us) {
            return -1;
        }
    }
    return absolute_time_diff_us(start, get_absolute_time());
}

void ir_init(uint8_t pin) {
    ir_pin = pin;
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

int ir_getkey(void) {
    // leader low (~9 ms)
    if (wait_for_level(ir_pin, 0, 150000) < 0) return -1;

    int64_t t = wait_for_level(ir_pin, 1, 12000);
    if (t < 8000 || t > 10000) return -1;

    t = wait_for_level(ir_pin, 0, 7000);
    if (t < 3500 || t > 5000) return -1;

    uint8_t data[4] = {0, 0, 0, 0};
    for (int i = 0; i < 32; ++i) {
        if (wait_for_level(ir_pin, 1, 1000) < 0) return -1;
        t = wait_for_level(ir_pin, 0, 2500);
        if (t < 200) return -1;
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        if (t > 1200) data[byte_idx] |= (1 << bit_idx); // logical '1'
    }

    // Validate address/data inverted pairs
    if ((uint8_t)(data[0] + data[1]) == 0xFF && (uint8_t)(data[2] + data[3]) == 0xFF) {
        return data[2];
    }
    return -1;
}

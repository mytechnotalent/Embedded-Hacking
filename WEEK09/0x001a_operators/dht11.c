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

static uint dht_pin;

void dht11_init(uint8_t pin) {
    dht_pin = pin;
    gpio_init(pin);
    gpio_pull_up(pin);
}

bool dht11_read(float *humidity, float *temperature) {
    uint8_t data[5] = {0};
    
    // Start signal
    gpio_set_dir(dht_pin, GPIO_OUT);
    gpio_put(dht_pin, 0);
    sleep_ms(18);
    gpio_put(dht_pin, 1);
    sleep_us(40);
    gpio_set_dir(dht_pin, GPIO_IN);
    
    // Wait for response
    uint32_t timeout = 10000;
    while (gpio_get(dht_pin) == 1) if (--timeout == 0) return false;
    timeout = 10000;
    while (gpio_get(dht_pin) == 0) if (--timeout == 0) return false;
    timeout = 10000;
    while (gpio_get(dht_pin) == 1) if (--timeout == 0) return false;
    
    // Read 40 bits
    for (int i = 0; i < 40; i++) {
        timeout = 10000;
        while (gpio_get(dht_pin) == 0) if (--timeout == 0) return false;
        uint32_t start = time_us_32();
        timeout = 10000;
        while (gpio_get(dht_pin) == 1) if (--timeout == 0) return false;
        uint32_t duration = time_us_32() - start;
        data[i / 8] <<= 1;
        if (duration > 40) data[i / 8] |= 1;
    }
    
    // Check checksum
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) return false;
    
    *humidity = data[0] + data[1] * 0.1f;
    *temperature = data[2] + data[3] * 0.1f;
    return true;
}

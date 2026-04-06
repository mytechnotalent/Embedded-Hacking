/**
 * @file button.c
 * @brief Implementation of the push-button GPIO input driver with debounce
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

#include "button.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/** @brief Debounce settling time stored from button_init() */
static uint32_t debounce_delay_ms = 20;

/**
 * @brief Confirm a raw active-low pin read by waiting for the debounce period
 *
 * After an initial low reading on the pin, this helper re-samples the pin
 * after debounce_delay_ms milliseconds to confirm the reading is stable.
 * This eliminates false triggers caused by mechanical contact bounce.
 *
 * @param pin GPIO pin number to re-sample
 * @return bool true if the pin is still low after the debounce delay
 */
static bool _debounce_confirm(uint32_t pin) {
    sleep_ms(debounce_delay_ms);
    return !gpio_get(pin);
}

void button_init(uint32_t pin, uint32_t debounce_ms) {
    debounce_delay_ms = debounce_ms;
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

bool button_is_pressed(uint32_t pin) {
    if (!gpio_get(pin)) {
        return _debounce_confirm(pin);
    }
    return false;
}

void button_led_init(uint32_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, false);
}

void button_led_set(uint32_t pin, bool on) {
    gpio_put(pin, on);
}

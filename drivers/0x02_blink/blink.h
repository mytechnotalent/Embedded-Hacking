/**
 * @file blink.h
 * @brief Header for GPIO output / LED blink driver
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

#ifndef BLINK_H
#define BLINK_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize a GPIO pin as a push-pull digital output
 *
 * Calls gpio_init() and gpio_set_dir() to configure the pin for output.
 * The initial drive level is low (LED off).
 *
 * @param pin GPIO pin number to configure as a digital output
 */
void blink_init(uint32_t pin);

/**
 * @brief Drive the output pin high (LED on)
 *
 * @param pin GPIO pin number previously initialized with blink_init()
 */
void blink_on(uint32_t pin);

/**
 * @brief Drive the output pin low (LED off)
 *
 * @param pin GPIO pin number previously initialized with blink_init()
 */
void blink_off(uint32_t pin);

/**
 * @brief Toggle the current state of the output pin
 *
 * Reads the current GPIO output level and inverts it. If the LED was on
 * it is turned off, and vice versa.
 *
 * @param pin GPIO pin number previously initialized with blink_init()
 */
void blink_toggle(uint32_t pin);

/**
 * @brief Query the current drive state of the output pin
 *
 * @param pin GPIO pin number previously initialized with blink_init()
 * @return bool true if the pin is driven high, false if driven low
 */
bool blink_get_state(uint32_t pin);

#endif // BLINK_H

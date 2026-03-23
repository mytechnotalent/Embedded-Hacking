/**
 * @file button.h
 * @brief Header for push-button GPIO input driver with debounce
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

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize a GPIO pin as an active-low button input with pull-up
 *
 * Configures the pin as an input and enables the internal pull-up resistor.
 * The pin reads logic high when the button is open and logic low when the
 * button connects the pin to GND.
 *
 * @param pin         GPIO pin number to configure as a button input
 * @param debounce_ms Debounce settling time in milliseconds (e.g. 20)
 */
void button_init(uint32_t pin, uint32_t debounce_ms);

/**
 * @brief Read the debounced state of the button
 *
 * Returns true only when the pin reads low continuously for the debounce
 * period configured in button_init(). This prevents mechanical contact
 * bounce from registering as multiple rapid presses.
 *
 * @param pin GPIO pin number previously initialized with button_init()
 * @return bool true if the button is firmly pressed, false if released
 */
bool button_is_pressed(uint32_t pin);

/**
 * @brief Initialize a GPIO pin as a push-pull digital output for an indicator LED
 *
 * Configures the pin as a digital output and drives it low (LED off). Use
 * together with button_led_set() to mirror button state visually.
 *
 * @param pin GPIO pin number to configure as an LED output
 */
void button_led_init(uint32_t pin);

/**
 * @brief Set the indicator LED state
 *
 * Drives the output pin high (LED on) or low (LED off).
 *
 * @param pin GPIO pin number previously initialized with button_led_init()
 * @param on  true to turn the LED on, false to turn it off
 */
void button_led_set(uint32_t pin, bool on);

#endif // BUTTON_H

/**
 * @file rp2350_button.h
 * @brief Button input driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Push-button GPIO input driver with software debounce.
 * The button pin is configured as active-low with internal
 * pull-up; pressing the button connects the pin to GND.
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
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
#ifndef __RP2350_BUTTON_H
#define __RP2350_BUTTON_H

#include "rp2350.h"

/**
  * @brief  Initialize a GPIO pin as an active-low button input with pull-up.
  * @param  pin         GPIO pin number to configure as a button input
  * @param  debounce_ms debounce settling time in milliseconds
  * @retval None
  */
void button_init(uint32_t pin, uint32_t debounce_ms);

/**
  * @brief  Read the debounced state of the button.
  * @param  pin GPIO pin number previously initialized with button_init()
  * @retval bool true if the button is firmly pressed, false if released
  */
bool button_is_pressed(uint32_t pin);

#endif /* __RP2350_BUTTON_H */

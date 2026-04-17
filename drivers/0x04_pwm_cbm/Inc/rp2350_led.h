/**
 * @file rp2350_led.h
 * @brief LED driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * High-level GPIO output / LED driver wrapping the
 * low-level GPIO functions.
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
#ifndef __RP2350_LED_H
#define __RP2350_LED_H

#include "rp2350.h"

/**
  * @brief  Initialize a GPIO pin as a push-pull digital output.
  * @param  pin GPIO pin number to configure
  * @retval None
  */
void led_init(uint32_t pin);

/**
  * @brief  Drive the output pin high (LED on).
  * @param  pin GPIO pin number
  * @retval None
  */
void led_on(uint32_t pin);

/**
  * @brief  Drive the output pin low (LED off).
  * @param  pin GPIO pin number
  * @retval None
  */
void led_off(uint32_t pin);

/**
  * @brief  Toggle the current state of the output pin.
  * @param  pin GPIO pin number
  * @retval None
  */
void led_toggle(uint32_t pin);

/**
  * @brief  Query the current drive state of the output pin.
  * @param  pin GPIO pin number
  * @retval bool true if the pin is driven high, false if low
  */
bool led_get_state(uint32_t pin);

#endif /* __RP2350_LED_H */

/**
 * @file pwm.h
 * @brief Header for generic PWM output driver
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

#ifndef PWM_H
#define PWM_H

#include <stdint.h>

/**
 * @brief Initialize PWM output on the specified GPIO pin at a target frequency
 *
 * Assigns the GPIO pin to the PWM function, calculates the clock divider and
 * wrap value from the system clock to achieve the requested frequency, then
 * starts the PWM slice with a 0% duty cycle. Must be called once before using
 * pwm_driver_set_duty_percent().
 *
 * @param pin     GPIO pin number to drive with PWM output
 * @param freq_hz Desired PWM frequency in Hz (e.g. 1000 for 1 kHz)
 */
void pwm_driver_init(uint32_t pin, uint32_t freq_hz);

/**
 * @brief Set the PWM duty cycle as an integer percentage
 *
 * Maps the percentage value to the internal PWM counter range and writes
 * the result to the channel level register. Values above 100 are clamped
 * to 100.
 *
 * @param percent Duty cycle from 0 (always low) to 100 (always high)
 */
void pwm_driver_set_duty_percent(uint8_t percent);

#endif // PWM_H

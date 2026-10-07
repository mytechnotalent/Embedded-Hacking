/**
 * @file servo.h
 * @brief Header for SG90 servo driver (PWM)
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

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize servo driver on a given GPIO pin
 *
 * Configures PWM for a 50 Hz servo (SG90). Call once before using other API.
 *
 * @param pin GPIO pin number to use for servo PWM
 */
void servo_init(uint8_t pin);

/**
 * @brief Set servo pulse width in microseconds (typical 1000-2000)
 *
 * @param pulse_us Pulse width in microseconds
 */
void servo_set_pulse_us(uint16_t pulse_us);

/**
 * @brief Set servo angle in degrees (0 to 180)
 *
 * Maps 0..180 degrees to the configured pulse range (default 1000..2000 us).
 * Values outside [0,180] will be clamped.
 *
 * @param degrees Angle in degrees
 */
void servo_set_angle(float degrees);

#endif // SERVO_H

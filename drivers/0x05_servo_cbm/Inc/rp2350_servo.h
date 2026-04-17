/**
 * @file rp2350_servo.h
 * @brief SG90 servo driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * PWM-based servo driver on GPIO 6 at 50 Hz. Supports pulse
 * width control in microseconds and angle control in degrees.
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
#ifndef __RP2350_SERVO_H
#define __RP2350_SERVO_H

#include "rp2350.h"

/**
  * @brief  Release PWM from reset and wait until ready.
  * @retval None
  */
void servo_release_reset(void);

/**
  * @brief  Initialize servo PWM on GPIO 6 at 50 Hz.
  * @retval None
  */
void servo_init(void);

/**
  * @brief  Set the servo pulse width in microseconds (clamped 1000-2000).
  * @param  pulse_us pulse width in microseconds
  * @retval None
  */
void servo_set_pulse_us(uint16_t pulse_us);

/**
  * @brief  Set the servo angle in degrees (clamped 0-180).
  * @param  degrees angle from 0 to 180
  * @retval None
  */
void servo_set_angle(uint8_t degrees);

#endif /* __RP2350_SERVO_H */

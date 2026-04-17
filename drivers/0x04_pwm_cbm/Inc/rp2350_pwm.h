/**
 * @file rp2350_pwm.h
 * @brief PWM driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Provides PWM output on GPIO 25 (onboard LED) at approximately
 * 1 kHz using PWM slice 4, channel B. Duty cycle is controllable
 * from 0 to 100 percent.
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
#ifndef __RP2350_PWM_H
#define __RP2350_PWM_H

#include "rp2350.h"

/**
  * @brief  Release PWM from reset and wait until ready.
  * @retval None
  */
void pwm_release_reset(void);

/**
  * @brief  Initialize PWM on GPIO 25 at approximately 1 kHz.
  * @retval None
  */
void pwm_init(void);

/**
  * @brief  Set the PWM duty cycle as an integer percentage.
  * @param  percent duty cycle from 0 (off) to 100 (fully on)
  * @retval None
  */
void pwm_set_duty(uint8_t percent);

#endif /* __RP2350_PWM_H */

/**
 * @file rp2350_timer.h
 * @brief TIMER0 alarm driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Bare-metal TIMER0 driver providing a repeating alarm
 * interrupt on alarm 0. The tick generator is configured
 * for 1 us resolution from the 12 MHz CLK_REF.
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
#ifndef __RP2350_TIMER_H
#define __RP2350_TIMER_H

#include "rp2350.h"

/**
  * @brief  Callback type for repeating timer alarm.
  */
typedef void (*timer_callback_t)(void);

/**
  * @brief  Release TIMER0 from reset and wait until ready.
  * @retval None
  */
void timer_release_reset(void);

/**
  * @brief  Start the TIMER0 tick generator at 1 us resolution.
  * @retval None
  */
void timer_tick_init(void);

/**
  * @brief  Start a repeating alarm that fires every period_ms milliseconds.
  * @param  period_ms interval in milliseconds between callbacks
  * @param  cb        function to call on each alarm
  * @retval None
  */
void timer_alarm_start(uint32_t period_ms, timer_callback_t cb);

#endif /* __RP2350_TIMER_H */

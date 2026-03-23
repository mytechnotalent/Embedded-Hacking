/**
 * @file timer.h
 * @brief Header for repeating hardware timer driver
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

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Timer callback function signature
 *
 * Must return true to keep the timer repeating, or false to stop.
 */
typedef bool (*timer_driver_callback_t)(void);

/**
 * @brief Start a repeating hardware timer that fires the given callback
 *
 * Schedules @p callback to be invoked every @p period_ms milliseconds using
 * the Pico SDK add_repeating_timer_ms() API. The timer continues firing
 * until timer_driver_cancel() is called.
 *
 * @param period_ms Interval between callbacks in milliseconds (positive value)
 * @param callback  Function to call on each timer expiry; must return true to
 *                  continue repeating, false to stop
 */
void timer_driver_start(int32_t period_ms, timer_driver_callback_t callback);

/**
 * @brief Cancel the active repeating timer
 *
 * Stops the timer started by timer_driver_start(). Safe to call even if the
 * timer has already fired and self-cancelled.
 */
void timer_driver_cancel(void);

#endif // TIMER_H

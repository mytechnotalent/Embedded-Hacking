/**
 * @file timer.c
 * @brief Implementation of repeating timer driver
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

#include "timer.h"
#include "pico/time.h"

/** @brief Pico SDK repeating timer handle */
static repeating_timer_t g_timer;
/** @brief Flag indicating whether the timer is running */
static bool g_timer_active = false;
/** @brief User-provided timer callback function */
static timer_driver_callback_t g_user_callback = NULL;

/**
 * @brief Internal repeating-timer callback that forwards to the user callback
 *
 * @param rt Unused repeating-timer handle provided by the SDK
 * @return bool true to keep the timer running, false to cancel
 */
static bool _timer_shim(repeating_timer_t *rt) {
    (void)rt;
    if (g_user_callback)
        return g_user_callback();
    return false;
}

/**
 * @brief Start a repeating hardware timer that fires the given callback
 *
 * Schedules callback to be invoked every period_ms milliseconds using
 * the Pico SDK add_repeating_timer_ms() API. The timer continues firing
 * until timer_driver_cancel() is called.
 *
 * @param period_ms Interval between callbacks in milliseconds (positive value)
 * @param callback  Function to call on each timer expiry; must return true to
 *                  continue repeating, false to stop
 */
void timer_driver_start(int32_t period_ms, timer_driver_callback_t callback) {
    if (g_timer_active) {
        cancel_repeating_timer(&g_timer);
        g_timer_active = false;
    }
    g_user_callback = callback;
    g_timer_active = add_repeating_timer_ms(period_ms, _timer_shim, NULL, &g_timer);
}

/**
 * @brief Cancel the active repeating timer
 *
 * Stops the timer started by timer_driver_start(). Safe to call even if the
 * timer has already fired and self-cancelled.
 */
void timer_driver_cancel(void) {
    if (!g_timer_active)
        return;
    cancel_repeating_timer(&g_timer);
    g_timer_active = false;
}

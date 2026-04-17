/**
 * @file rp2350_watchdog.h
 * @brief Watchdog timer driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Bare-metal watchdog driver providing enable, feed, and
 * reboot-cause detection. The watchdog tick generator is
 * configured for 1 us resolution from the 12 MHz CLK_REF.
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
#ifndef __RP2350_WATCHDOG_H
#define __RP2350_WATCHDOG_H

#include "rp2350.h"

/**
  * @brief  Start the watchdog tick generator at 1 us resolution.
  * @retval None
  */
void watchdog_tick_init(void);

/**
  * @brief  Enable the watchdog with the specified timeout.
  * @param  timeout_ms watchdog timeout in milliseconds (1-16777)
  * @retval None
  */
void watchdog_enable(uint32_t timeout_ms);

/**
  * @brief  Feed the watchdog to prevent a reset.
  * @retval None
  */
void watchdog_feed(void);

/**
  * @brief  Check whether the last reset was caused by the watchdog.
  * @retval bool true if the watchdog triggered the last reset
  */
bool watchdog_caused_reboot(void);

#endif /* __RP2350_WATCHDOG_H */

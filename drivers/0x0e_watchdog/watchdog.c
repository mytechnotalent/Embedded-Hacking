/**
 * @file watchdog.c
 * @brief Implementation of hardware watchdog driver
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

#include "watchdog.h"
#include "hardware/watchdog.h"

/**
 * @brief Enable the hardware watchdog with the specified timeout
 *
 * Starts the watchdog timer. If watchdog_driver_feed() is not called within
 * timeout_ms milliseconds, the RP2350 will perform a hard reset.
 * The maximum supported timeout is 8388 ms.
 *
 * @param timeout_ms Watchdog timeout in milliseconds (1 - 8388)
 */
void watchdog_driver_enable(uint32_t timeout_ms) {
    watchdog_enable(timeout_ms, true);
}

/**
 * @brief Reset ("feed") the watchdog timer to prevent a reboot
 *
 * Must be called periodically within the timeout window configured by
 * watchdog_driver_enable(). Each call restarts the countdown.
 */
void watchdog_driver_feed(void) {
    watchdog_update();
}

/**
 * @brief Check whether the last reset was caused by the watchdog
 *
 * Reads the reset reason register to determine if the watchdog timer
 * expired and forced the most recent reboot.
 *
 * @return bool true if the watchdog triggered the last reset, false otherwise
 */
bool watchdog_driver_caused_reboot(void) {
    return watchdog_caused_reboot();
}

/**
 * @file 0x0d_timer.c
 * @brief Repeating timer demo using timer.c/timer.h
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
 *
 * -----------------------------------------------------------------------------
 *
 * Demonstrates repeating timer callbacks using the timer driver
 * (timer.h / timer.c). A one-second heartbeat timer prints a message
 * over UART to confirm the timer is firing.
 *
 * Wiring:
 *   No external wiring required
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "timer.h"

/**
 * @brief Heartbeat timer callback that prints a message over UART
 *
 * @return bool true to keep the repeating timer active
 */
static bool _heartbeat_callback(void) {
    printf("Timer heartbeat\r\n");
    return true;
}

int main(void) {
    stdio_init_all();
    timer_driver_start(1000, _heartbeat_callback);
    while (true)
        tight_loop_contents();
}

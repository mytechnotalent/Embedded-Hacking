/**
 * @file 0x0e_watchdog.c
 * @brief Watchdog feed demo using watchdog.c/watchdog.h
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
 * Demonstrates the hardware watchdog using the watchdog driver
 * (watchdog.h / watchdog.c). The watchdog is enabled with a 3-second
 * timeout and fed every second. If the feed loop were removed, the
 * chip would automatically reboot after 3 seconds.
 *
 * Wiring:
 *   No external wiring required
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "watchdog.h"

/**
 * @brief Print whether the system booted normally or from a watchdog reset
 */
static void _print_reset_reason(void) {
    if (watchdog_driver_caused_reboot())
        printf("System rebooted by watchdog timeout\r\n");
    else
        printf("Normal power-on reset\r\n");
}


/**
 * @brief Feed the watchdog and log over UART, then wait 1 second
 */
static void _feed_and_report(void) {
    watchdog_driver_feed();
    printf("Watchdog fed\r\n");
    sleep_ms(1000);
}


/**
 * @brief Application entry point for the watchdog demo
 *
 * Enables the watchdog with a 3-second timeout and feeds it every
 * 1 second. Reports the reset reason on startup.
 *
 * @return int Does not return
 */
int main(void) {
    stdio_init_all();
    _print_reset_reason();
    watchdog_driver_enable(3000);
    printf("Watchdog enabled (3s timeout). Feeding every 1s...\r\n");
    while (true)
        _feed_and_report();
}

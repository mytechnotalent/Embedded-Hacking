/**
 * @file main.c
 * @brief Blink demonstration: toggle onboard LED every 500 ms
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
 * Demonstrates GPIO output control using the blink driver (blink.h / blink.c).
 * The onboard LED on GPIO 25 is toggled every BLINK_DELAY_MS milliseconds and
 * the current state is reported over UART.
 *
 * Wiring:
 *   GPIO25 -> Onboard LED (no external wiring needed)
 */

#include "blink.h"
#include "uart.h"
#include "delay.h"
#include "constants.h"

#define BLINK_DELAY_MS 500

/**
 * @brief Print the current LED state over UART
 *
 * @details Queries the blink driver for the pin state and prints
 *          "LED: ON\r\n" or "LED: OFF\r\n" over UART0.
 *
 * @param pin GPIO pin number to query
 */
static void _print_led_state(uint32_t pin) {
    if (blink_get_state(pin)) {
        uart_driver_puts("LED: ON\r\n");
    } else {
        uart_driver_puts("LED: OFF\r\n");
    }
}

/**
 * @brief Application entry point for the LED blink demo
 *
 * Initializes the onboard LED and enters an infinite loop that
 * toggles the LED state every BLINK_DELAY_MS milliseconds.
 *
 * @return int Does not return
 */
int main(void) {
    blink_init(LED_PIN);
    uart_driver_puts("Blink driver initialized on GPIO 25\r\n");
    while (1) {
        blink_toggle(LED_PIN);
        _print_led_state(LED_PIN);
        Delay_MS(BLINK_DELAY_MS);
    }
}

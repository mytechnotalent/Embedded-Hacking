/**
 * @file 0x0008_unitialized-variables-b.c
 * @brief Blink LED using gpioc coprocessor bit-level functions
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
 * Blinks an LED on GPIO16 using the RP2350 gpioc coprocessor bit-level
 * functions (gpioc_bit_oe_put / gpioc_bit_out_put) instead of the standard
 * gpio_init / gpio_put SDK calls.
 *
 * Wiring:
 *   GPIO16 -> LED anode (with current-limiting resistor to GND)
 */

#include <stdio.h>
#include "pico/stdlib.h"

/** @brief GPIO pin number for the LED */
#define LED_PIN 16

/**
 * @brief Initialize GPIO16 for output using coprocessor bit functions
 *
 * @details Configures the pad for SIO, sets direction to input first,
 *          clears the output, selects SIO function, then enables output.
 *
 * @retval None
 */
static void gpio_coprocessor_init(void) {
    gpio_set_dir(LED_PIN, GPIO_IN);
    gpio_put(LED_PIN, 0);
    gpio_set_function(LED_PIN, GPIO_FUNC_SIO);
    gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
}

/**
 * @brief Blink LED using coprocessor bit output functions
 *
 * @details Toggles the LED on and off with 500ms delays using
 *          gpioc_bit_out_put and sleep_us.
 *
 * @retval None
 */
static void blink_cycle(void) {
    gpioc_bit_out_put(LED_PIN, 1);
    sleep_us(500 * 1000ull);
    gpioc_bit_out_put(LED_PIN, 0);
    sleep_us(500 * 1000ull);
}

int main(void) {
    gpio_coprocessor_init();
    while (true) {
        blink_cycle();
    }
}

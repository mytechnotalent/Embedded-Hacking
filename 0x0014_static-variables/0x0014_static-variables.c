/**
 * @file 0x0014_static-variables.c
 * @brief Static variables: compare regular vs static local variable persistence
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
 * Demonstrates the difference between regular and static local variables.
 * A regular variable resets to 42 each iteration while a static variable
 * persists and increments across loop iterations. Also reads a button on
 * GPIO15 and mirrors its state to an LED on GPIO16.
 *
 * Wiring:
 *   GPIO15 -> Button (with pull-up, active low)
 *   GPIO16 -> LED anode (with current-limiting resistor to GND)
 */

#include <stdio.h>
#include "pico/stdlib.h"

/** @brief GPIO pin number for the button input */
#define BUTTON_GPIO 15
/** @brief GPIO pin number for the LED output */
#define LED_GPIO    16

/**
 * @brief Initialize button and LED GPIO pins
 *
 * @details Configures the button pin as input with pull-up and the
 *          LED pin as output.
 *
 * @retval None
 */
static void init_gpio(void) {
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
}

/**
 * @brief Print and increment regular vs static variables, update LED
 *
 * @details Prints both variable values, increments them, reads the
 *          button state and drives the LED accordingly.
 *
 * @retval None
 */
static void demo_static_variable(void) {
    uint8_t regular_fav_num = 42;
    static uint8_t static_fav_num = 42;
    printf("regular_fav_num: %d\r\n", regular_fav_num);
    printf("static_fav_num: %d\r\n", static_fav_num);
    regular_fav_num++;
    static_fav_num++;
    bool pressed = gpio_get(BUTTON_GPIO);
    gpio_put(LED_GPIO, pressed ? 0 : 1);
}

int main(void) {
    stdio_init_all();
    init_gpio();
    while (true) {
        demo_static_variable();
    }
}

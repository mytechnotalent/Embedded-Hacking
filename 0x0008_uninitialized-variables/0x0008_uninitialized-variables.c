/**
 * @file 0x0008_uninitialized-variables.c
 * @brief Uninitialized variables: demonstrate undefined behavior with printf
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
 * Demonstrates the danger of uninitialized variables. Prints the value of an
 * uninitialized uint8_t over UART while blinking an LED on GPIO16.
 *
 * Wiring:
 *   GPIO16 -> LED anode (with current-limiting resistor to GND)
 */

#include <stdio.h>
#include "pico/stdlib.h"

/** @brief GPIO pin number for the LED */
#define LED_PIN 16

/**
 * @brief Toggle LED and print uninitialized variable value
 *
 * @details Blinks the LED on and off with a 500ms delay between each
 *          transition and prints the age variable each cycle.
 *
 * @param age value to print (uninitialized in this demo)
 */
static void blink_and_print(uint8_t age) {
    printf("age: %d\r\n", age);
    gpio_put(LED_PIN, 1);
    sleep_ms(500);
    gpio_put(LED_PIN, 0);
    sleep_ms(500);
}

int main(void) {
    uint8_t age;
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        blink_and_print(age);
    }
}

/**
 * @file 0x001d_static-conditionals.c
 * @brief Static conditionals: if/else and switch with servo sweep
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
 * Demonstrates static (compile-time known) conditionals using if/else if/else
 * and switch/case statements. Also sweeps a servo on GPIO6 between 0 and
 * 180 degrees each iteration.
 *
 * Wiring:
 *   GPIO6 -> Servo signal wire (orange/white)
 *   5V    -> Servo VCC (red)
 *   GND   -> Servo GND (brown/black)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.h"

/** @brief GPIO pin number for the servo */
#define SERVO_GPIO 6

/**
 * @brief Print choice value using if/else if/else conditional
 *
 * @details Prints "1", "2", or "?" depending on the choice value.
 *
 * @param choice integer value to evaluate
 * @retval None
 */
static void print_if_else(int choice) {
    if (choice == 1) {
        printf("1\r\n");
    } else if (choice == 2) {
        printf("2\r\n");
    } else {
        printf("?\r\n");
    }
}

/**
 * @brief Print choice value using switch/case conditional
 *
 * @details Prints "one", "two", or "??" depending on the choice value.
 *
 * @param choice integer value to evaluate
 * @retval None
 */
static void print_switch(int choice) {
    switch (choice) {
        case 1:  printf("one\r\n"); break;
        case 2:  printf("two\r\n"); break;
        default: printf("??\r\n");
    }
}

/**
 * @brief Sweep servo between 0 and 180 degrees
 *
 * @details Sets the servo to 0 degrees, waits 500ms, then sets
 *          to 180 degrees and waits 500ms.
 *
 * @retval None
 */
static void sweep_servo(void) {
    servo_set_angle(0.0f);
    sleep_ms(500);
    servo_set_angle(180.0f);
    sleep_ms(500);
}

int main(void) {
    stdio_init_all();
    int choice = 1;
    servo_init(SERVO_GPIO);
    while (true) {
        print_if_else(choice);
        print_switch(choice);
        sweep_servo();
    }
}
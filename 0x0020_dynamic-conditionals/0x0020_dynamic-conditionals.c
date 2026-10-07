/**
 * @file 0x0020_dynamic-conditionals.c
 * @brief Dynamic conditionals: UART input drives if/else and switch with servo
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
 * Demonstrates dynamic (runtime) conditionals using UART input via getchar().
 * Reads a character, evaluates it with if/else and switch/case, and controls
 * a servo on GPIO6 based on the input ('1' or '2').
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
 * @brief Evaluate choice with if/else conditional and print result
 *
 * @details Compares against hex ASCII values 0x31 ('1') and 0x32 ('2').
 *
 * @param choice character value received from UART
 * @retval None
 */
static void eval_if_else(uint8_t choice) {
    if (choice == 0x31) {
        printf("1\r\n");
    } else if (choice == 0x32) {
        printf("2\r\n");
    } else {
        printf("??\r\n");
    }
}

/**
 * @brief Sweep servo from start angle to end angle with a pause
 *
 * @details Sets the servo to start, waits 500 ms, sets to end,
 *          waits 500 ms.
 *
 * @param label text to print before sweeping
 * @param start starting angle in degrees
 * @param end   ending angle in degrees
 * @retval None
 */
static void sweep_servo(const char *label, float start, float end) {
    printf("%s\r\n", label);
    servo_set_angle(start);
    sleep_ms(500);
    servo_set_angle(end);
    sleep_ms(500);
}

/**
 * @brief Process choice with switch/case and drive servo accordingly
 *
 * @details On '1', sweeps servo 0->180; on '2', sweeps 180->0;
 *          otherwise prints unknown.
 *
 * @param choice character value received from UART
 * @retval None
 */
static void process_servo_command(uint8_t choice) {
    switch (choice) {
        case '1': sweep_servo("one", 0.0f, 180.0f); break;
        case '2': sweep_servo("two", 180.0f, 0.0f); break;
        default:  printf("??\r\n");
    }
}

int main(void) {
    stdio_init_all();
    uint8_t choice = 0;
    servo_init(SERVO_GPIO);
    while (true) {
        choice = getchar();
        eval_if_else(choice);
        process_servo_command(choice);
    }
}

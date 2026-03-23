/**
 * @file 0x05_servo.c
 * @brief SG90 servo motor driver for the Raspberry Pi Pico 2
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
 * This driver demonstrates SG90 servo control using the servo.c/servo.h
 * driver (PWM at 50 Hz on GPIO 6). The servo sweeps from 0 degrees to
 * 180 degrees and back in 10-degree increments, printing each angle over
 * UART.
 *
 * Wiring:
 *   GPIO6  -> Servo signal wire (orange or yellow)
 *   5V     -> Servo power wire  (red)   -- use external 5 V supply for load
 *   GND    -> Servo ground wire (brown or black)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.h"

#define SERVO_GPIO     6
#define STEP_DEGREES   10
#define STEP_DELAY_MS  150


/**
 * @brief Sweep the servo between start and end angles in given steps
 *
 * Iterates from start to end with the given step increment, setting
 * each angle and printing the current position with a delay between steps.
 *
 * @param start Starting angle in degrees
 * @param end   Ending angle in degrees
 * @param step  Increment per iteration (negative for descending)
 */
static void _sweep_angle(int start, int end, int step) {
    for (int angle = start; (step > 0) ? angle <= end : angle >= end; angle += step) {
        servo_set_angle((float)angle);
        printf("Angle: %3d deg\r\n", angle);
        sleep_ms(STEP_DELAY_MS);
    }
}


/**
 * @brief Application entry point for the servo sweep demo
 *
 * Initializes the servo on GPIO and continuously sweeps 0-180-0 degrees
 * in STEP_DEGREES increments, reporting each angle over UART.
 *
 * @return int Does not return
 */
int main(void) {
    stdio_init_all();
    servo_init(SERVO_GPIO);
    printf("Servo driver initialized on GPIO %d\r\n", SERVO_GPIO);
    printf("Sweeping 0 -> 180 -> 0 degrees in %d-degree steps\r\n", STEP_DEGREES);
    while (true) {
        _sweep_angle(0, 180, STEP_DEGREES);
        _sweep_angle(180, 0, -STEP_DEGREES);
    }
}
    }
}

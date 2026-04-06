/**
 * @file 0x04_pwm.c
 * @brief PWM demonstration: LED breathing effect via duty-cycle sweep
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
 * Demonstrates PWM output using the pwm driver (pwm.h / pwm.c). A 1 kHz
 * signal on GPIO 25 (onboard LED) sweeps its duty cycle from 0% to 100%
 * and back to produce a smooth breathing effect. The current duty is
 * reported over UART at 115200 baud.
 *
 * Wiring:
 *   GPIO0  -> UART TX (USB-to-UART adapter RX)
 *   GPIO1  -> UART RX (USB-to-UART adapter TX)
 *   GPIO25 -> Onboard LED (no external wiring needed)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pwm.h"

/** @brief GPIO pin for PWM output */
#define PWM_PIN     25
/** @brief Target PWM frequency in Hz */
#define PWM_FREQ_HZ 1000

/**
 * @brief Sweep the PWM duty cycle between start and end in given steps
 *
 * Iterates from start to end with the given step increment, updating
 * the PWM duty cycle and printing each value with a 50 ms delay.
 *
 * @param start Starting duty percentage
 * @param end   Ending duty percentage
 * @param step  Increment per iteration (negative for descending)
 */
static void _sweep_duty(int start, int end, int step) {
    for (int duty = start; (step > 0) ? duty <= end : duty >= end; duty += step) {
        pwm_driver_set_duty_percent((uint8_t)duty);
        printf("Duty: %3d%%\r\n", duty);
        sleep_ms(50);
    }
}

int main(void) {
    stdio_init_all();
    pwm_driver_init(PWM_PIN, PWM_FREQ_HZ);
    printf("PWM initialized: GPIO%d @ %d Hz\r\n", PWM_PIN, PWM_FREQ_HZ);
    while (true) {
        _sweep_duty(0, 100, 5);
        _sweep_duty(100, 0, -5);
    }
}

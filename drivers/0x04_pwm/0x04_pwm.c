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
 * signal on GPIO 0 sweeps its duty cycle from 0% to 100% and back to
 * produce a smooth LED breathing effect. The current duty is reported
 * over UART.
 *
 * Wiring:
 *   GPIO0 -> LED anode (with 330 ohm series resistor to GND)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pwm.h"

#define PWM_PIN     0
#define PWM_FREQ_HZ 1000

int main(void) {
    stdio_init_all();
    pwm_driver_init(PWM_PIN, PWM_FREQ_HZ);

    printf("PWM driver initialized: GPIO%d @ %d Hz\r\n", PWM_PIN, PWM_FREQ_HZ);

    while (true) {
        for (int duty = 0; duty <= 100; duty += 5) {
            pwm_driver_set_duty_percent((uint8_t)duty);
            printf("Duty: %3d%%\r\n", duty);
            sleep_ms(50);
        }
        for (int duty = 100; duty >= 0; duty -= 5) {
            pwm_driver_set_duty_percent((uint8_t)duty);
            printf("Duty: %3d%%\r\n", duty);
            sleep_ms(50);
        }
    }
}

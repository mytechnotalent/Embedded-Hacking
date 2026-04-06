/**
 * @file pwm.c
 * @brief Implementation of the generic PWM output driver
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
 */

#include "pwm.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

/** @brief PWM hardware slice index */
static uint     pwm_slice;
/** @brief PWM channel (A or B) within the slice */
static uint     pwm_chan;
/** @brief PWM counter wrap value */
static uint32_t pwm_wrap;

/**
 * @brief Compute the PWM clock divider that yields the target frequency
 *
 * Derives a floating-point divider from the current system clock frequency,
 * the desired output frequency, and the chosen wrap value so that the PWM
 * counter overflows exactly freq_hz times per second.
 *
 * @param freq_hz  Desired PWM output frequency in Hz
 * @param wrap_val Chosen PWM counter wrap value (period - 1)
 * @return float   Clock divider to program into the PWM slice
 */
static float _calc_clk_div(uint32_t freq_hz, uint32_t wrap_val) {
    uint32_t sys_hz = clock_get_hz(clk_sys);
    return (float)sys_hz / ((float)freq_hz * (float)(wrap_val + 1));
}

/**
 * @brief Apply the PWM configuration to the active slice
 *
 * Builds a default config, sets the clock divider for the target frequency,
 * programs the wrap value, starts the slice, and zeroes the channel level.
 *
 * @param freq_hz Desired PWM output frequency in Hz
 */
static void _apply_pwm_config(uint32_t freq_hz) {
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, _calc_clk_div(freq_hz, pwm_wrap));
    pwm_config_set_wrap(&cfg, pwm_wrap);
    pwm_init(pwm_slice, &cfg, true);
    pwm_set_chan_level(pwm_slice, pwm_chan, 0);
}

/**
 * @brief Initialize PWM output on the specified GPIO pin
 *
 * @param pin     GPIO pin number to drive with PWM output
 * @param freq_hz Desired PWM frequency in Hz
 */
void pwm_driver_init(uint32_t pin, uint32_t freq_hz) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(pin);
    pwm_chan  = pwm_gpio_to_channel(pin);
    pwm_wrap  = 10000 - 1;
    _apply_pwm_config(freq_hz);
}

/**
 * @brief Set the PWM duty cycle as an integer percentage
 *
 * @param percent Duty cycle from 0 (always low) to 100 (always high)
 */
void pwm_driver_set_duty_percent(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    uint32_t level = ((uint32_t)percent * (pwm_wrap + 1)) / 100;
    pwm_set_chan_level(pwm_slice, pwm_chan, level);
}

/**
 * @file servo.c
 * @brief Implementation of a simple SG90 servo driver using PWM (50Hz)
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

#include "servo.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

static const uint16_t SERVO_DEFAULT_MIN_US = 1000;
static const uint16_t SERVO_DEFAULT_MAX_US = 2000;

static uint8_t servo_pin = 0;
static uint servo_slice = 0;
static uint servo_chan = 0;
static uint32_t servo_wrap = 20000 - 1;
static float servo_hz = 50.0f;
static bool servo_initialized = false;


/**
 * @brief Convert a pulse width in microseconds to a PWM counter level
 *
 * Uses the configured PWM wrap and servo frequency to map pulse time
 * into the channel compare value expected by the PWM hardware.
 *
 * @param pulse_us Pulse width in microseconds
 * @return uint32_t PWM level suitable for pwm_set_chan_level()
 */
static uint32_t _pulse_us_to_level(uint32_t pulse_us) {
    const float period_us = 1000000.0f / servo_hz;
    float counts_per_us = (servo_wrap + 1) / period_us;
    return (uint32_t)(pulse_us * counts_per_us + 0.5f);
}


/**
 * @brief Build and apply the PWM slice configuration for 50 Hz servo
 *
 * Computes the clock divider from the system clock to achieve the
 * target servo frequency with the chosen wrap value, then starts
 * the PWM slice.
 */
static void _apply_servo_config(void) {
    pwm_config config = pwm_get_default_config();
    const uint32_t sys_clock_hz = clock_get_hz(clk_sys);
    float clock_div = (float)sys_clock_hz / (servo_hz * (servo_wrap + 1));
    pwm_config_set_clkdiv(&config, clock_div);
    pwm_config_set_wrap(&config, servo_wrap);
    pwm_init(servo_slice, &config, true);
}


void servo_init(uint8_t pin) {
    servo_pin = pin;
    gpio_set_function(servo_pin, GPIO_FUNC_PWM);
    servo_slice = pwm_gpio_to_slice_num(servo_pin);
    servo_chan = pwm_gpio_to_channel(servo_pin);
    _apply_servo_config();
    servo_initialized = true;
}


void servo_set_pulse_us(uint16_t pulse_us) {
    if (!servo_initialized) return;
    if (pulse_us < SERVO_DEFAULT_MIN_US) pulse_us = SERVO_DEFAULT_MIN_US;
    if (pulse_us > SERVO_DEFAULT_MAX_US) pulse_us = SERVO_DEFAULT_MAX_US;
    uint32_t level = _pulse_us_to_level(pulse_us);
    pwm_set_chan_level(servo_slice, servo_chan, level);
}


void servo_set_angle(float degrees) {
    if (degrees < 0.0f) degrees = 0.0f;
    if (degrees > 180.0f) degrees = 180.0f;
    float ratio = degrees / 180.0f;
    uint16_t pulse = (uint16_t)(SERVO_DEFAULT_MIN_US + ratio * (SERVO_DEFAULT_MAX_US - SERVO_DEFAULT_MIN_US) + 0.5f);
    servo_set_pulse_us(pulse);
}
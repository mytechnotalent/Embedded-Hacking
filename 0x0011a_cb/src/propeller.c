// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    propeller.c
// Desc:    Implements SG90 servo PWM mock propeller control on GPIO6.
// Created: 2026

#include "propeller.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

static struct repeating_timer prop_timer;
static bool prop_active = false;
static uint16_t current_pulse = 1000;

void init_propeller(void)
{
    gpio_set_function(PROPELLER_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PROPELLER_PIN);
    pwm_set_clkdiv(slice, 150.0f);
    pwm_set_wrap(slice, 19999);
    pwm_set_gpio_level(PROPELLER_PIN, 1500);
    pwm_set_enabled(slice, true);
}

/**
 * @brief Repeating timer callback to alternate servo angle at max slew rate.
 *
 * @param t Pointer to repeating timer structure.
 * @return bool Always true to continue recurring timer.
 */
static bool prop_timer_callback(struct repeating_timer *t)
{
    (void)t;
    current_pulse = (current_pulse == 1000) ? 2000 : 1000;
    pwm_set_gpio_level(PROPELLER_PIN, current_pulse);
    return true;
}

void propeller_spin(void)
{
    if (!prop_active) {
        prop_active = true;
        add_repeating_timer_ms(-150, prop_timer_callback, NULL, &prop_timer);
    }
}

void propeller_stop(void)
{
    if (prop_active) {
        cancel_repeating_timer(&prop_timer);
        prop_active = false;
    }
    pwm_set_gpio_level(PROPELLER_PIN, 0);
}

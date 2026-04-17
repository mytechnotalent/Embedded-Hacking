/**
 * @file rp2350_led.c
 * @brief LED driver implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * High-level wrapper around the GPIO driver for LED control.
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
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
#include "rp2350_led.h"
#include "rp2350_gpio.h"

void led_init(uint32_t pin)
{
  gpio_config(pin);
  gpio_clear(pin);
}

void led_on(uint32_t pin)
{
  gpio_set(pin);
}

void led_off(uint32_t pin)
{
  gpio_clear(pin);
}

void led_toggle(uint32_t pin)
{
  gpio_toggle(pin);
}

bool led_get_state(uint32_t pin)
{
  return gpio_get(pin);
}

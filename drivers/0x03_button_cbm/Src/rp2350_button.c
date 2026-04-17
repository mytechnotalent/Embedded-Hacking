/**
 * @file rp2350_button.c
 * @brief Button input driver implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Configures a GPIO pin as an active-low input with internal
 * pull-up and provides debounced press detection using a
 * busy-wait confirmation delay.
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
#include "rp2350_button.h"
#include "rp2350_gpio.h"
#include "rp2350_delay.h"

static uint32_t debounce_delay_ms = 20;

/**
  * @brief  Re-sample the pin after the debounce delay to confirm press.
  * @param  pin GPIO pin number to re-sample
  * @retval bool true if the pin is still low after the debounce delay
  */
static bool debounce_confirm(uint32_t pin)
{
  delay_ms(debounce_delay_ms);
  return !gpio_get(pin);
}

void button_init(uint32_t pin, uint32_t debounce_ms)
{
  debounce_delay_ms = debounce_ms;
  gpio_config_input_pullup(pin);
}

bool button_is_pressed(uint32_t pin)
{
  if (!gpio_get(pin))
    return debounce_confirm(pin);
  return false;
}

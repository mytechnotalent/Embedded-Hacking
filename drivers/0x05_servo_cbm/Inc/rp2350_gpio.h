/**
 * @file rp2350_gpio.h
 * @brief GPIO driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * SIO-based GPIO configuration, set, clear, toggle, and read
 * functions for the RP2350 GPIO pins 0-29.
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
#ifndef __RP2350_GPIO_H
#define __RP2350_GPIO_H

#include "rp2350.h"

/**
  * @brief  Configure a GPIO pin as SIO output.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_config(uint32_t gpio_num);

/**
  * @brief  Drive a GPIO output high.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_set(uint32_t gpio_num);

/**
  * @brief  Drive a GPIO output low.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_clear(uint32_t gpio_num);

/**
  * @brief  Toggle a GPIO output.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_toggle(uint32_t gpio_num);

/**
  * @brief  Read the current input level of a GPIO pin.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval bool true if pin is high, false if low
  */
bool gpio_get(uint32_t gpio_num);

#endif /* __RP2350_GPIO_H */

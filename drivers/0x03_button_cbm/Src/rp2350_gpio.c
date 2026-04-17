/**
 * @file rp2350_gpio.c
 * @brief GPIO driver implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * SIO-based GPIO configuration using IO_BANK0 and PADS_BANK0
 * register structs defined in rp2350.h. All register offsets
 * verified against the RP2350 datasheet (RP-008373-DS-2).
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
#include "rp2350_gpio.h"

/**
  * @brief  Configure pad control for a GPIO pin.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
static void gpio_config_pad(uint32_t gpio_num)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[gpio_num];
  value &= ~(1U << PADS_BANK0_OD_SHIFT);
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  PADS_BANK0->GPIO[gpio_num] = value;
}

/**
  * @brief  Set IO_BANK0 FUNCSEL to SIO for a GPIO pin.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
static void gpio_config_funcsel(uint32_t gpio_num)
{
  uint32_t value;
  value = IO_BANK0->GPIO[gpio_num].CTRL;
  value &= ~IO_BANK0_CTRL_FUNCSEL_MASK;
  value |= IO_BANK0_CTRL_FUNCSEL_SIO;
  IO_BANK0->GPIO[gpio_num].CTRL = value;
}

/**
  * @brief  Configure pad for input with pull-up enabled.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
static void gpio_config_pad_input_pullup(uint32_t gpio_num)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[gpio_num];
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  value |= (1U << PADS_BANK0_PUE_SHIFT);
  value &= ~(1U << PADS_BANK0_PDE_SHIFT);
  PADS_BANK0->GPIO[gpio_num] = value;
}

/**
  * @brief  Enable the output driver for a GPIO pin via SIO.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
static void gpio_enable_output(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OE_SET_OFFSET] = (1U << gpio_num);
}

void gpio_config(uint32_t gpio_num)
{
  gpio_config_pad(gpio_num);
  gpio_config_funcsel(gpio_num);
  gpio_enable_output(gpio_num);
}

void gpio_set(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OUT_SET_OFFSET] = (1U << gpio_num);
}

void gpio_clear(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OUT_CLR_OFFSET] = (1U << gpio_num);
}

void gpio_toggle(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OUT_XOR_OFFSET] = (1U << gpio_num);
}

bool gpio_get(uint32_t gpio_num)
{
  return (SIO[SIO_GPIO_IN_OFFSET] & (1U << gpio_num)) != 0;
}

void gpio_config_input_pullup(uint32_t gpio_num)
{
  gpio_config_pad_input_pullup(gpio_num);
  gpio_config_funcsel(gpio_num);
}

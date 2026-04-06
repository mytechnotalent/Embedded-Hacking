/**
  ******************************************************************************
  * @file    rp2350_gpio.c
  * @author  Kevin Thomas
  * @brief   GPIO driver implementation for RP2350.
  *
  *          SIO-based GPIO configuration using IO_BANK0 and PADS_BANK0
  *          register structs defined in rp2350.h. All register offsets
  *          verified against the RP2350 datasheet (RP-008373-DS-2).
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "rp2350_gpio.h"

/**
  * @brief  Configure pad control for a GPIO pin.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
static void _gpio_config_pad(uint32_t gpio_num)
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
static void _gpio_config_funcsel(uint32_t gpio_num)
{
  uint32_t value;
  value = IO_BANK0->GPIO[gpio_num].CTRL;
  value &= ~IO_BANK0_CTRL_FUNCSEL_MASK;
  value |= IO_BANK0_CTRL_FUNCSEL_SIO;
  IO_BANK0->GPIO[gpio_num].CTRL = value;
}

/**
  * @brief  Enable the output driver for a GPIO pin via SIO.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
static void _gpio_enable_output(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OE_SET_OFFSET] = (1U << gpio_num);
}

/**
  * @brief  Configure a GPIO pin as SIO output.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_config(uint32_t gpio_num)
{
  _gpio_config_pad(gpio_num);
  _gpio_config_funcsel(gpio_num);
  _gpio_enable_output(gpio_num);
}

/**
  * @brief  Drive a GPIO output high.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_set(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OUT_SET_OFFSET] = (1U << gpio_num);
}

/**
  * @brief  Drive a GPIO output low.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_clear(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OUT_CLR_OFFSET] = (1U << gpio_num);
}

/**
  * @brief  Toggle a GPIO output.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval None
  */
void gpio_toggle(uint32_t gpio_num)
{
  SIO[SIO_GPIO_OUT_XOR_OFFSET] = (1U << gpio_num);
}

/**
  * @brief  Read the current input level of a GPIO pin.
  * @param  gpio_num GPIO pin number (0-29)
  * @retval bool true if pin is high, false if low
  */
bool gpio_get(uint32_t gpio_num)
{
  return (SIO[SIO_GPIO_IN_OFFSET] & (1U << gpio_num)) != 0;
}

/**
  ******************************************************************************
  * @file    rp2350_gpio.h
  * @author  Kevin Thomas
  * @brief   GPIO driver header for RP2350.
  *
  *          SIO-based GPIO configuration, set, clear, toggle, and read
  *          functions for the RP2350 GPIO pins 0-29.
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

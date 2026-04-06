/**
  ******************************************************************************
  * @file    rp2350_led.c
  * @author  Kevin Thomas
  * @brief   LED driver implementation for RP2350.
  *
  *          High-level wrapper around the GPIO driver for LED control.
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

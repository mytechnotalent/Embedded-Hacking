/**
  ******************************************************************************
  * @file    rp2350_button.c
  * @author  Kevin Thomas
  * @brief   Button input driver implementation for RP2350.
  *
  *          Configures a GPIO pin as an active-low input with internal
  *          pull-up and provides debounced press detection using a
  *          busy-wait confirmation delay.
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

#include "rp2350_button.h"
#include "rp2350_gpio.h"
#include "rp2350_delay.h"

static uint32_t debounce_delay_ms = 20;

/**
  * @brief  Re-sample the pin after the debounce delay to confirm press.
  * @param  pin GPIO pin number to re-sample
  * @retval bool true if the pin is still low after the debounce delay
  */
static bool _debounce_confirm(uint32_t pin)
{
  delay_ms(debounce_delay_ms);
  return !gpio_get(pin);
}

/**
  * @brief  Initialize a GPIO pin as an active-low button input with pull-up.
  * @param  pin         GPIO pin number to configure as a button input
  * @param  debounce_ms debounce settling time in milliseconds
  * @retval None
  */
void button_init(uint32_t pin, uint32_t debounce_ms)
{
  debounce_delay_ms = debounce_ms;
  gpio_config_input_pullup(pin);
}

/**
  * @brief  Read the debounced state of the button.
  * @param  pin GPIO pin number previously initialized with button_init()
  * @retval bool true if the button is firmly pressed, false if released
  */
bool button_is_pressed(uint32_t pin)
{
  if (!gpio_get(pin))
    return _debounce_confirm(pin);
  return false;
}

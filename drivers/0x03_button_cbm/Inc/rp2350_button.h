/**
  ******************************************************************************
  * @file    rp2350_button.h
  * @author  Kevin Thomas
  * @brief   Button input driver header for RP2350.
  *
  *          Push-button GPIO input driver with software debounce.
  *          The button pin is configured as active-low with internal
  *          pull-up; pressing the button connects the pin to GND.
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

#ifndef __RP2350_BUTTON_H
#define __RP2350_BUTTON_H

#include "rp2350.h"

/**
  * @brief  Initialize a GPIO pin as an active-low button input with pull-up.
  * @param  pin         GPIO pin number to configure as a button input
  * @param  debounce_ms debounce settling time in milliseconds
  * @retval None
  */
void button_init(uint32_t pin, uint32_t debounce_ms);

/**
  * @brief  Read the debounced state of the button.
  * @param  pin GPIO pin number previously initialized with button_init()
  * @retval bool true if the button is firmly pressed, false if released
  */
bool button_is_pressed(uint32_t pin);

#endif /* __RP2350_BUTTON_H */

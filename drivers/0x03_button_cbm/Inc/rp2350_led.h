/**
  ******************************************************************************
  * @file    rp2350_led.h
  * @author  Kevin Thomas
  * @brief   LED driver header for RP2350.
  *
  *          High-level GPIO output / LED driver wrapping the
  *          low-level GPIO functions.
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

#ifndef __RP2350_LED_H
#define __RP2350_LED_H

#include "rp2350.h"

/**
  * @brief  Initialize a GPIO pin as a push-pull digital output.
  * @param  pin GPIO pin number to configure
  * @retval None
  */
void led_init(uint32_t pin);

/**
  * @brief  Drive the output pin high (LED on).
  * @param  pin GPIO pin number
  * @retval None
  */
void led_on(uint32_t pin);

/**
  * @brief  Drive the output pin low (LED off).
  * @param  pin GPIO pin number
  * @retval None
  */
void led_off(uint32_t pin);

/**
  * @brief  Toggle the current state of the output pin.
  * @param  pin GPIO pin number
  * @retval None
  */
void led_toggle(uint32_t pin);

/**
  * @brief  Query the current drive state of the output pin.
  * @param  pin GPIO pin number
  * @retval bool true if the pin is driven high, false if low
  */
bool led_get_state(uint32_t pin);

#endif /* __RP2350_LED_H */

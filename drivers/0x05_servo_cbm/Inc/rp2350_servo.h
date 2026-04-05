/**
  ******************************************************************************
  * @file    rp2350_servo.h
  * @author  Kevin Thomas
  * @brief   SG90 servo driver header for RP2350.
  *
  *          PWM-based servo driver on GPIO 6 at 50 Hz. Supports pulse
  *          width control in microseconds and angle control in degrees.
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

#ifndef __RP2350_SERVO_H
#define __RP2350_SERVO_H

#include "rp2350.h"

/**
  * @brief  Release PWM from reset and wait until ready.
  * @retval None
  */
void servo_release_reset(void);

/**
  * @brief  Initialize servo PWM on GPIO 6 at 50 Hz.
  * @retval None
  */
void servo_init(void);

/**
  * @brief  Set the servo pulse width in microseconds (clamped 1000-2000).
  * @param  pulse_us pulse width in microseconds
  * @retval None
  */
void servo_set_pulse_us(uint16_t pulse_us);

/**
  * @brief  Set the servo angle in degrees (clamped 0-180).
  * @param  degrees angle from 0 to 180
  * @retval None
  */
void servo_set_angle(uint8_t degrees);

#endif /* __RP2350_SERVO_H */

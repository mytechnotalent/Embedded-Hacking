/**
  ******************************************************************************
  * @file    rp2350_pwm.h
  * @author  Kevin Thomas
  * @brief   PWM driver header for RP2350.
  *
  *          Provides PWM output on GPIO 25 (onboard LED) at approximately
  *          1 kHz using PWM slice 4, channel B. Duty cycle is controllable
  *          from 0 to 100 percent.
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

#ifndef __RP2350_PWM_H
#define __RP2350_PWM_H

#include "rp2350.h"

/**
  * @brief  Release PWM from reset and wait until ready.
  * @retval None
  */
void pwm_release_reset(void);

/**
  * @brief  Initialize PWM on GPIO 25 at approximately 1 kHz.
  * @retval None
  */
void pwm_init(void);

/**
  * @brief  Set the PWM duty cycle as an integer percentage.
  * @param  percent duty cycle from 0 (off) to 100 (fully on)
  * @retval None
  */
void pwm_set_duty(uint8_t percent);

#endif /* __RP2350_PWM_H */

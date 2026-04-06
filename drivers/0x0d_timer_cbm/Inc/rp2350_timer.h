/**
  ******************************************************************************
  * @file    rp2350_timer.h
  * @author  Kevin Thomas
  * @brief   TIMER0 alarm driver header for RP2350.
  *
  *          Bare-metal TIMER0 driver providing a repeating alarm
  *          interrupt on alarm 0. The tick generator is configured
  *          for 1 us resolution from the 12 MHz CLK_REF.
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

#ifndef __RP2350_TIMER_H
#define __RP2350_TIMER_H

#include "rp2350.h"

/**
  * @brief  Callback type for repeating timer alarm.
  */
typedef void (*timer_callback_t)(void);

/**
  * @brief  Release TIMER0 from reset and wait until ready.
  * @retval None
  */
void timer_release_reset(void);

/**
  * @brief  Start the TIMER0 tick generator at 1 us resolution.
  * @retval None
  */
void timer_tick_init(void);

/**
  * @brief  Start a repeating alarm that fires every period_ms milliseconds.
  * @param  period_ms interval in milliseconds between callbacks
  * @param  cb        function to call on each alarm
  * @retval None
  */
void timer_alarm_start(uint32_t period_ms, timer_callback_t cb);

#endif /* __RP2350_TIMER_H */

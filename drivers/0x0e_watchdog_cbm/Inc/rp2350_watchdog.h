/**
  ******************************************************************************
  * @file    rp2350_watchdog.h
  * @author  Kevin Thomas
  * @brief   Watchdog timer driver header for RP2350.
  *
  *          Bare-metal watchdog driver providing enable, feed, and
  *          reboot-cause detection. The watchdog tick generator is
  *          configured for 1 us resolution from the 12 MHz CLK_REF.
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

#ifndef __RP2350_WATCHDOG_H
#define __RP2350_WATCHDOG_H

#include "rp2350.h"

/**
  * @brief  Start the watchdog tick generator at 1 us resolution.
  * @retval None
  */
void watchdog_tick_init(void);

/**
  * @brief  Enable the watchdog with the specified timeout.
  * @param  timeout_ms watchdog timeout in milliseconds (1-16777)
  * @retval None
  */
void watchdog_enable(uint32_t timeout_ms);

/**
  * @brief  Feed the watchdog to prevent a reset.
  * @retval None
  */
void watchdog_feed(void);

/**
  * @brief  Check whether the last reset was caused by the watchdog.
  * @retval bool true if the watchdog triggered the last reset
  */
bool watchdog_caused_reboot(void);

#endif /* __RP2350_WATCHDOG_H */

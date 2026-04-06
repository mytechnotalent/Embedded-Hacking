/**
  ******************************************************************************
  * @file    rp2350_watchdog.c
  * @author  Kevin Thomas
  * @brief   Watchdog timer driver implementation for RP2350.
  *
  *          Configures the watchdog tick generator for 1 us resolution
  *          from the 12 MHz CLK_REF, enables the watchdog countdown,
  *          and provides feed and reboot-cause detection.
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

#include "rp2350_watchdog.h"

/**
  * @brief  Saved load value for feeding the watchdog.
  */
static uint32_t _load_value;

/**
  * @brief  Set the watchdog tick generator cycle count to 12.
  * @retval None
  */
static void _watchdog_set_tick_cycles(void)
{
  TICKS_WATCHDOG->CYCLES = TICKS_CYCLES_12MHZ;
}

/**
  * @brief  Enable the watchdog tick generator.
  * @retval None
  */
static void _watchdog_enable_tick(void)
{
  TICKS_WATCHDOG->CTRL = (1U << TICKS_CTRL_ENABLE_SHIFT);
}

/**
  * @brief  Disable the watchdog before reconfiguring.
  * @retval None
  */
static void _watchdog_disable(void)
{
  WATCHDOG->CTRL &= ~(1U << WATCHDOG_CTRL_ENABLE_SHIFT);
}

/**
  * @brief  Configure PSM WDSEL to reset everything except oscillators.
  * @retval None
  */
static void _watchdog_set_psm_wdsel(void)
{
  uint32_t mask;
  mask = PSM_WDSEL_ALL_MASK;
  mask &= ~(1U << PSM_WDSEL_ROSC_SHIFT);
  mask &= ~(1U << PSM_WDSEL_XOSC_SHIFT);
  PSM->WDSEL = mask;
}

/**
  * @brief  Set pause-on-debug bits in CTRL so debugger halts the timer.
  * @retval None
  */
static void _watchdog_set_pause_debug(void)
{
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_PAUSE_DBG0_SHIFT);
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_PAUSE_DBG1_SHIFT);
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_PAUSE_JTAG_SHIFT);
}

/**
  * @brief  Load and enable the watchdog countdown.
  * @retval None
  */
static void _watchdog_load_and_enable(void)
{
  WATCHDOG->LOAD = _load_value;
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_ENABLE_SHIFT);
}

void watchdog_tick_init(void)
{
  _watchdog_set_tick_cycles();
  _watchdog_enable_tick();
}

void watchdog_enable(uint32_t timeout_ms)
{
  _load_value = timeout_ms * 1000U;
  if (_load_value > WATCHDOG_LOAD_MAX)
    _load_value = WATCHDOG_LOAD_MAX;
  _watchdog_disable();
  _watchdog_set_psm_wdsel();
  _watchdog_set_pause_debug();
  _watchdog_load_and_enable();
}

void watchdog_feed(void)
{
  WATCHDOG->LOAD = _load_value;
}

bool watchdog_caused_reboot(void)
{
  return (WATCHDOG->REASON & ((1U << WATCHDOG_REASON_TIMER_SHIFT) | (1U << WATCHDOG_REASON_FORCE_SHIFT))) != 0;
}

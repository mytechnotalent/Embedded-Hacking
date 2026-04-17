/**
 * @file rp2350_watchdog.c
 * @brief Watchdog timer driver implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Configures the watchdog tick generator for 1 us resolution
 * from the 12 MHz CLK_REF, enables the watchdog countdown,
 * and provides feed and reboot-cause detection.
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "rp2350_watchdog.h"

/**
  * @brief  Saved load value for feeding the watchdog.
  */
static uint32_t load_value;

/**
  * @brief  Set the watchdog tick generator cycle count to 12.
  * @retval None
  */
static void watchdog_set_tick_cycles(void)
{
  TICKS_WATCHDOG->CYCLES = TICKS_CYCLES_12MHZ;
}

/**
  * @brief  Enable the watchdog tick generator.
  * @retval None
  */
static void watchdog_enable_tick(void)
{
  TICKS_WATCHDOG->CTRL = (1U << TICKS_CTRL_ENABLE_SHIFT);
}

/**
  * @brief  Disable the watchdog before reconfiguring.
  * @retval None
  */
static void watchdog_disable(void)
{
  WATCHDOG->CTRL &= ~(1U << WATCHDOG_CTRL_ENABLE_SHIFT);
}

/**
  * @brief  Configure PSM WDSEL to reset everything except oscillators.
  * @retval None
  */
static void watchdog_set_psm_wdsel(void)
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
static void watchdog_set_pause_debug(void)
{
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_PAUSE_DBG0_SHIFT);
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_PAUSE_DBG1_SHIFT);
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_PAUSE_JTAG_SHIFT);
}

/**
  * @brief  Load and enable the watchdog countdown.
  * @retval None
  */
static void watchdog_load_and_enable(void)
{
  WATCHDOG->LOAD = load_value;
  WATCHDOG->CTRL |= (1U << WATCHDOG_CTRL_ENABLE_SHIFT);
}

void watchdog_tick_init(void)
{
  watchdog_set_tick_cycles();
  watchdog_enable_tick();
}

void watchdog_enable(uint32_t timeout_ms)
{
  load_value = timeout_ms * 1000U;
  if (load_value > WATCHDOG_LOAD_MAX)
    load_value = WATCHDOG_LOAD_MAX;
  watchdog_disable();
  watchdog_set_psm_wdsel();
  watchdog_set_pause_debug();
  watchdog_load_and_enable();
}

void watchdog_feed(void)
{
  WATCHDOG->LOAD = load_value;
}

bool watchdog_caused_reboot(void)
{
  return (WATCHDOG->REASON & ((1U << WATCHDOG_REASON_TIMER_SHIFT) | (1U << WATCHDOG_REASON_FORCE_SHIFT))) != 0;
}

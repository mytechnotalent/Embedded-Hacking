/**
  ******************************************************************************
  * @file    rp2350_timer.c
  * @author  Kevin Thomas
  * @brief   TIMER0 alarm driver implementation for RP2350.
  *
  *          Configures the TIMER0 tick generator for 1 us resolution
  *          from the 12 MHz CLK_REF, then uses alarm 0 with NVIC IRQ
  *          to implement a repeating callback at a configurable period.
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

#include "rp2350_timer.h"

/**
  * @brief  User callback and period stored for ISR re-arm.
  */
static timer_callback_t _user_callback;

/**
  * @brief  Alarm period in microseconds for re-arming.
  */
static uint32_t _alarm_period_us;

/**
  * @brief  Clear the TIMER0 reset bit in the reset controller.
  * @retval None
  */
static void _timer_clear_reset_bit(void)
{
  uint32_t value;
  value = RESETS->RESET;
  value &= ~(1U << RESETS_RESET_TIMER0_SHIFT);
  RESETS->RESET = value;
}

/**
  * @brief  Wait until the TIMER0 block is out of reset.
  * @retval None
  */
static void _timer_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_TIMER0_SHIFT)) == 0) {
  }
}

/**
  * @brief  Set the TIMER0 tick generator cycle count to 12.
  * @retval None
  */
static void _timer_set_tick_cycles(void)
{
  TICKS_TIMER0->CYCLES = TICKS_TIMER0_CYCLES_12MHZ;
}

/**
  * @brief  Enable the TIMER0 tick generator.
  * @retval None
  */
static void _timer_enable_tick(void)
{
  TICKS_TIMER0->CTRL = (1U << TICKS_CTRL_ENABLE_SHIFT);
}

/**
  * @brief  Enable the alarm 0 interrupt in TIMER0 INTE register.
  * @retval None
  */
static void _timer_enable_alarm_irq(void)
{
  TIMER0->INTE = (1U << TIMER_INTE_ALARM0_SHIFT);
}

/**
  * @brief  Enable TIMER0_IRQ_0 in the NVIC.
  * @retval None
  */
static void _timer_enable_nvic(void)
{
  *NVIC_ISER0 = (1U << TIMER0_ALARM0_IRQ);
}

/**
  * @brief  Arm alarm 0 with the next target time.
  * @retval None
  */
static void _timer_arm_alarm(void)
{
  uint32_t target;
  target = TIMER0->TIMERAWL + _alarm_period_us;
  TIMER0->ALARM0 = target;
}

void timer_release_reset(void)
{
  _timer_clear_reset_bit();
  _timer_wait_reset_done();
}

void timer_tick_init(void)
{
  _timer_set_tick_cycles();
  _timer_enable_tick();
}

void timer_alarm_start(uint32_t period_ms, timer_callback_t cb)
{
  _user_callback = cb;
  _alarm_period_us = period_ms * 1000U;
  _timer_enable_alarm_irq();
  _timer_enable_nvic();
  _timer_arm_alarm();
}

/**
  * @brief  TIMER0 alarm 0 interrupt handler (exception 16 + IRQ 0).
  * @retval None
  */
void TIMER0_IRQ_0_Handler(void)
{
  TIMER0->INTR = TIMER_INTR_ALARM0_MASK;
  _timer_arm_alarm();
  if (_user_callback)
    _user_callback();
}

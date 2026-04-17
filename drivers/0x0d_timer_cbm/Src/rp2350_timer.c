/**
 * @file rp2350_timer.c
 * @brief TIMER0 alarm driver implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Configures the TIMER0 tick generator for 1 us resolution
 * from the 12 MHz CLK_REF, then uses alarm 0 with NVIC IRQ
 * to implement a repeating callback at a configurable period.
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
#include "rp2350_timer.h"

/**
  * @brief  User callback and period stored for ISR re-arm.
  */
static timer_callback_t user_callback;

/**
  * @brief  Alarm period in microseconds for re-arming.
  */
static uint32_t alarm_period_us;

/**
  * @brief  Clear the TIMER0 reset bit in the reset controller.
  * @retval None
  */
static void timer_clear_reset_bit(void)
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
static void timer_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_TIMER0_SHIFT)) == 0) {}
}

/**
  * @brief  Set the TIMER0 tick generator cycle count to 12.
  * @retval None
  */
static void timer_set_tick_cycles(void)
{
  TICKS_TIMER0->CYCLES = TICKS_TIMER0_CYCLES_12MHZ;
}

/**
  * @brief  Enable the TIMER0 tick generator.
  * @retval None
  */
static void timer_enable_tick(void)
{
  TICKS_TIMER0->CTRL = (1U << TICKS_CTRL_ENABLE_SHIFT);
}

/**
  * @brief  Enable the alarm 0 interrupt in TIMER0 INTE register.
  * @retval None
  */
static void timer_enable_alarm_irq(void)
{
  TIMER0->INTE = (1U << TIMER_INTE_ALARM0_SHIFT);
}

/**
  * @brief  Enable TIMER0_IRQ_0 in the NVIC.
  * @retval None
  */
static void timer_enable_nvic(void)
{
  *NVIC_ISER0 = (1U << TIMER0_ALARM0_IRQ);
}

/**
  * @brief  Arm alarm 0 with the next target time.
  * @retval None
  */
static void timer_arm_alarm(void)
{
  uint32_t target;
  target = TIMER0->TIMERAWL + alarm_period_us;
  TIMER0->ALARM0 = target;
}

void timer_release_reset(void)
{
  timer_clear_reset_bit();
  timer_wait_reset_done();
}

void timer_tick_init(void)
{
  timer_set_tick_cycles();
  timer_enable_tick();
}

void timer_alarm_start(uint32_t period_ms, timer_callback_t cb)
{
  user_callback = cb;
  alarm_period_us = period_ms * 1000U;
  timer_enable_alarm_irq();
  timer_enable_nvic();
  timer_arm_alarm();
}

void TIMER0_IRQ_0_Handler(void)
{
  TIMER0->INTR = TIMER_INTR_ALARM0_MASK;
  timer_arm_alarm();
  if (user_callback)
    user_callback();
}

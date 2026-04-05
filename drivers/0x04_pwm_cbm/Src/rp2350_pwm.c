/**
  ******************************************************************************
  * @file    rp2350_pwm.c
  * @author  Kevin Thomas
  * @brief   PWM driver implementation for RP2350.
  *
  *          Configures PWM slice 4, channel B on GPIO 25 (onboard LED).
  *          Uses a wrap value of 9999 with no clock division, yielding
  *          approximately 650 Hz from the ROSC system clock.
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

#include "rp2350_pwm.h"

#define PWM ((volatile uint32_t *) PWM_BASE)

/**
  * @brief  PWM slice number for GPIO 25.
  */
#define PWM_SLICE  4U

/**
  * @brief  Compute the byte offset for a register within the slice.
  * @param  reg per-slice register offset (e.g. PWM_CH_CSR_OFFSET)
  * @retval uint32_t word index from PWM_BASE
  */
#define PWM_REG(reg) (((PWM_SLICE * PWM_CH_STRIDE) + (reg)) / 4U)

/**
  * @brief  Clear the PWM reset bit in the reset controller.
  * @retval None
  */
static void _pwm_clear_reset_bit(void)
{
  uint32_t value;
  value = RESETS->RESET;
  value &= ~(1U << RESETS_RESET_PWM_SHIFT);
  RESETS->RESET = value;
}

/**
  * @brief  Wait until the PWM block is out of reset.
  * @retval None
  */
static void _pwm_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_PWM_SHIFT)) == 0) {
  }
}

/**
  * @brief  Configure GPIO 25 pad and funcsel for PWM output.
  * @retval None
  */
static void _pwm_configure_pin(void)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[LED_PIN];
  value &= ~(1U << PADS_BANK0_OD_SHIFT);
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  PADS_BANK0->GPIO[LED_PIN] = value;
  IO_BANK0->GPIO[LED_PIN].CTRL = IO_BANK0_CTRL_FUNCSEL_PWM;
}

/**
  * @brief  Set the clock divider to 1 (no division).
  * @retval None
  */
static void _pwm_set_divider(void)
{
  PWM[PWM_REG(PWM_CH_DIV_OFFSET)] = (1U << PWM_DIV_INT_SHIFT);
}

/**
  * @brief  Set the wrap (TOP) value and zero the compare register.
  * @retval None
  */
static void _pwm_set_wrap(void)
{
  PWM[PWM_REG(PWM_CH_TOP_OFFSET)] = PWM_WRAP_DEFAULT;
  PWM[PWM_REG(PWM_CH_CC_OFFSET)] = 0;
}

/**
  * @brief  Enable the PWM slice counter.
  * @retval None
  */
static void _pwm_enable(void)
{
  PWM[PWM_REG(PWM_CH_CSR_OFFSET)] = (1U << PWM_CSR_EN_SHIFT);
}

void pwm_release_reset(void)
{
  _pwm_clear_reset_bit();
  _pwm_wait_reset_done();
}

void pwm_init(void)
{
  _pwm_configure_pin();
  _pwm_set_divider();
  _pwm_set_wrap();
  _pwm_enable();
}

void pwm_set_duty(uint8_t percent)
{
  uint32_t level;
  if (percent > 100)
    percent = 100;
  level = ((uint32_t)percent * (PWM_WRAP_DEFAULT + 1)) / 100;
  PWM[PWM_REG(PWM_CH_CC_OFFSET)] = (level << PWM_CC_B_SHIFT);
}

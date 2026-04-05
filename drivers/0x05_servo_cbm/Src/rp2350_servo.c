/**
  ******************************************************************************
  * @file    rp2350_servo.c
  * @author  Kevin Thomas
  * @brief   SG90 servo driver implementation for RP2350.
  *
  *          Configures PWM slice 3, channel A on GPIO 6 at 50 Hz for
  *          standard hobby servo control. Uses a wrap of 19999 and a
  *          fractional clock divider of 14.5 (14.5 MHz XOSC / 50 Hz /
  *          20000 = 14.5).
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

#include "rp2350_servo.h"

#define PWM ((volatile uint32_t *) PWM_BASE)

/**
  * @brief  PWM slice number for GPIO 6.
  */
#define SERVO_SLICE   3U

/**
  * @brief  Wrap value for 50 Hz at 14.5 MHz with divider 14.5.
  */
#define SERVO_WRAP    19999U

/**
  * @brief  Clock divider register value for 14.5 (INT=14, FRAC=8).
  */
#define SERVO_DIV_VAL ((14U << PWM_DIV_INT_SHIFT) | 8U)

/**
  * @brief  Minimum pulse width in microseconds (0 degrees).
  */
#define SERVO_MIN_US  1000U

/**
  * @brief  Maximum pulse width in microseconds (180 degrees).
  */
#define SERVO_MAX_US  2000U

/**
  * @brief  Compute the byte offset for a register within the slice.
  * @param  reg per-slice register offset (e.g. PWM_CH_CSR_OFFSET)
  * @retval uint32_t word index from PWM_BASE
  */
#define SERVO_REG(reg) (((SERVO_SLICE * PWM_CH_STRIDE) + (reg)) / 4U)

/**
  * @brief  Clear the PWM reset bit in the reset controller.
  * @retval None
  */
static void _servo_clear_reset_bit(void)
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
static void _servo_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_PWM_SHIFT)) == 0) {
  }
}

/**
  * @brief  Configure GPIO 6 pad and funcsel for PWM output.
  * @retval None
  */
static void _servo_configure_pin(void)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[SERVO_PIN];
  value &= ~(1U << PADS_BANK0_OD_SHIFT);
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  PADS_BANK0->GPIO[SERVO_PIN] = value;
  IO_BANK0->GPIO[SERVO_PIN].CTRL = IO_BANK0_CTRL_FUNCSEL_PWM;
}

/**
  * @brief  Set the fractional clock divider for 50 Hz servo frequency.
  * @retval None
  */
static void _servo_set_divider(void)
{
  PWM[SERVO_REG(PWM_CH_DIV_OFFSET)] = SERVO_DIV_VAL;
}

/**
  * @brief  Set the wrap value and zero the compare register.
  * @retval None
  */
static void _servo_set_wrap(void)
{
  PWM[SERVO_REG(PWM_CH_TOP_OFFSET)] = SERVO_WRAP;
  PWM[SERVO_REG(PWM_CH_CC_OFFSET)] = 0;
}

/**
  * @brief  Enable the PWM slice counter.
  * @retval None
  */
static void _servo_enable(void)
{
  PWM[SERVO_REG(PWM_CH_CSR_OFFSET)] = (1U << PWM_CSR_EN_SHIFT);
}

/**
  * @brief  Convert a pulse width in microseconds to a PWM counter level.
  * @param  pulse_us pulse width in microseconds
  * @retval uint32_t PWM level for the CC register
  */
static uint32_t _pulse_us_to_level(uint16_t pulse_us)
{
  return ((uint32_t)pulse_us * (SERVO_WRAP + 1)) / 20000U;
}

void servo_release_reset(void)
{
  _servo_clear_reset_bit();
  _servo_wait_reset_done();
}

void servo_init(void)
{
  _servo_configure_pin();
  _servo_set_divider();
  _servo_set_wrap();
  _servo_enable();
}

void servo_set_pulse_us(uint16_t pulse_us)
{
  uint32_t level;
  if (pulse_us < SERVO_MIN_US)
    pulse_us = SERVO_MIN_US;
  if (pulse_us > SERVO_MAX_US)
    pulse_us = SERVO_MAX_US;
  level = _pulse_us_to_level(pulse_us);
  PWM[SERVO_REG(PWM_CH_CC_OFFSET)] = level;
}

void servo_set_angle(uint8_t degrees)
{
  uint16_t pulse;
  if (degrees > 180)
    degrees = 180;
  pulse = (uint16_t)(SERVO_MIN_US + ((uint32_t)degrees * (SERVO_MAX_US - SERVO_MIN_US)) / 180U);
  servo_set_pulse_us(pulse);
}

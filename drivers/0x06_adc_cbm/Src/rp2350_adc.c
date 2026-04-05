/**
  ******************************************************************************
  * @file    rp2350_adc.c
  * @author  Kevin Thomas
  * @brief   RP2350 12-bit ADC driver implementation.
  *
  *          Bare-metal driver for the RP2350 ADC peripheral. Reads analog
  *          voltage on GPIO26 (channel 0) and the on-chip temperature sensor
  *          (channel 4). All register accesses verified against the RP2350
  *          datasheet (RP-008373-DS-2).
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

#include "rp2350_adc.h"

static uint8_t active_channel = 0;

/**
  * @brief  Configure the GPIO26 pad for analog input.
  *
  *         Sets drive strength to 4 mA and clears pad isolation, output
  *         disable, input enable, pull-up, pull-down, and schmitt trigger
  *         bits so the pad is fully analog.
  *
  * @retval None
  */
static void _adc_config_pad(void)
{
  PADS_BANK0->GPIO[ADC_PIN] = PADS_BANK0_DRIVE_4MA;
}

/**
  * @brief  Set GPIO26 IO mux function to NULL (disconnected).
  *
  *         Writes FUNCSEL = 0x1F to the IO_BANK0 control register for
  *         GPIO26, ensuring no digital peripheral drives the pin.
  *
  * @retval None
  */
static void _adc_config_gpio(void)
{
  IO_BANK0->GPIO[ADC_PIN].CTRL = IO_BANK0_CTRL_FUNCSEL_NULL;
}

/**
  * @brief  Select an ADC input channel.
  *
  *         Writes the channel number into the AINSEL field of the CS
  *         register while preserving all other bits.
  *
  * @param  ch channel number (0-4)
  * @retval None
  */
static void _adc_select_input(uint8_t ch)
{
  uint32_t cs = ADC->CS;
  cs &= ~ADC_CS_AINSEL_MASK;
  cs |= ((uint32_t)ch << ADC_CS_AINSEL_SHIFT);
  ADC->CS = cs;
}

/**
  * @brief  Trigger a single conversion and return the raw 12-bit result.
  *
  *         Sets START_ONCE in the CS register, spins until READY is
  *         asserted, then reads the 12-bit value from the RESULT register.
  *
  * @retval uint16_t raw ADC conversion result (0-4095)
  */
static uint16_t _adc_read_raw(void)
{
  uint32_t timeout = ADC_READY_TIMEOUT;
  ADC->CS |= (1U << ADC_CS_START_ONCE_SHIFT);
  while (!(ADC->CS & (1U << ADC_CS_READY_SHIFT)) && timeout > 0U) {
    timeout--;
  }
  if (timeout == 0U) {
    return 0U;
  }
  return (uint16_t)(ADC->RESULT & 0xFFFU);
}

/**
  * @brief  Convert a raw 12-bit ADC value to millivolts.
  *
  *         Scales the raw value linearly against the 3.3 V (3300 mV)
  *         full-scale reference.
  *
  * @param  raw 12-bit ADC conversion result (0-4095)
  * @retval uint32_t equivalent voltage in millivolts (0-3300)
  */
static uint32_t _raw_to_mv(uint16_t raw)
{
  return (uint32_t)raw * ADC_VREF_MV / ADC_FULL_SCALE;
}

/**
  * @brief  Convert a raw temperature-sensor ADC value to tenths of degrees.
  *
  *         Applies the RP2350 datasheet formula T = 27 - (V - 0.706) / 0.001721
  *         using integer arithmetic with millivolt-times-ten precision.
  *         V is computed as raw * 33000 / 4095 (voltage in 0.1 mV units).
  *
  * @param  raw 12-bit ADC result from the internal temperature sensor
  * @retval int32_t die temperature in tenths of degrees Celsius
  */
static int32_t _raw_to_temp_tenths(uint16_t raw)
{
  int32_t v_mv10 = (int32_t)((uint32_t)raw * 33000U / ADC_FULL_SCALE);
  return 270 - (v_mv10 - 7060) * 1000 / 1721;
}

void adc_release_reset(void)
{
  RESETS->RESET |= (1U << RESETS_RESET_ADC_SHIFT);
  uint32_t timeout = ADC_READY_TIMEOUT;
  RESETS->RESET &= ~(1U << RESETS_RESET_ADC_SHIFT);
  while (!(RESETS->RESET_DONE & (1U << RESETS_RESET_ADC_SHIFT)) && timeout > 0U) {
    timeout--;
  }
}

void adc_init(void)
{
  _adc_config_pad();
  _adc_config_gpio();
  ADC->CS = (1U << ADC_CS_EN_SHIFT);
  uint32_t timeout = ADC_READY_TIMEOUT;
  while (!(ADC->CS & (1U << ADC_CS_READY_SHIFT)) && timeout > 0U) {
    timeout--;
  }
  ADC->CS |= (1U << ADC_CS_TS_EN_SHIFT);
  active_channel = ADC_CHANNEL;
  _adc_select_input(active_channel);
}

uint32_t adc_read_mv(void)
{
  return _raw_to_mv(_adc_read_raw());
}

int32_t adc_read_temp_tenths(void)
{
  _adc_select_input(ADC_TEMP_CHANNEL);
  int32_t result = _raw_to_temp_tenths(_adc_read_raw());
  _adc_select_input(active_channel);
  return result;
}

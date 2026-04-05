/**
  ******************************************************************************
  * @file    rp2350_adc.h
  * @author  Kevin Thomas
  * @brief   Header for RP2350 12-bit ADC driver.
  *
  *          Provides functions to initialise the ADC peripheral, read an
  *          analog voltage in millivolts from GPIO26 (channel 0), and read
  *          the on-chip temperature sensor (channel 4) in tenths of degrees
  *          Celsius.
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

#ifndef __RP2350_ADC_H
#define __RP2350_ADC_H

#include "rp2350.h"

/**
  * @brief  Release the ADC subsystem from reset.
  *
  *         Clears the ADC bit in the RESETS register and waits until
  *         RESET_DONE confirms the subsystem is running.
  *
  * @retval None
  */
void adc_release_reset(void);

/**
  * @brief  Initialise the ADC peripheral for GPIO26 (channel 0).
  *
  *         Configures GPIO26 pad for analog input (disables digital I/O,
  *         pulls, and pad isolation), sets the IO mux function to NULL,
  *         powers on the ADC, enables the temperature sensor, and selects
  *         channel 0 as the default input.
  *
  * @retval None
  */
void adc_init(void);

/**
  * @brief  Perform a single ADC conversion and return millivolts.
  *
  *         Triggers a one-shot conversion on the currently selected channel,
  *         waits for the result, and scales the 12-bit value against the
  *         3.3 V reference.
  *
  * @retval uint32_t measured voltage in millivolts (0-3300)
  */
uint32_t adc_read_mv(void);

/**
  * @brief  Read the on-chip temperature sensor in tenths of degrees Celsius.
  *
  *         Temporarily switches to ADC channel 4 (temperature sensor),
  *         performs a conversion, applies the RP2350 datasheet formula
  *         T = 27 - (V - 0.706) / 0.001721 using integer arithmetic,
  *         and restores the previously active channel.
  *
  * @retval int32_t die temperature in tenths of degrees (e.g. 270 = 27.0 C)
  */
int32_t adc_read_temp_tenths(void);

#endif /* __RP2350_ADC_H */

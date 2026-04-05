/**
  ******************************************************************************
  * @file    rp2350_delay.h
  * @author  Kevin Thomas
  * @brief   Delay driver header for RP2350.
  *
  *          Millisecond busy-wait delay calibrated for a 12 MHz clock.
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

#ifndef __RP2350_DELAY_H
#define __RP2350_DELAY_H

#include "rp2350.h"

/**
  * @brief  Delay for the specified number of milliseconds.
  * @param  ms number of milliseconds to delay
  * @retval None
  */
void delay_ms(uint32_t ms);

#endif /* __RP2350_DELAY_H */

/**
  ******************************************************************************
  * @file    rp2350_xosc.h
  * @author  Kevin Thomas
  * @brief   XOSC driver header for RP2350.
  *
  *          External crystal oscillator initialization and peripheral
  *          clock enable using the XOSC registers.
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

#ifndef __RP2350_XOSC_H
#define __RP2350_XOSC_H

#include "rp2350.h"

/**
  * @brief  Initialize the external crystal oscillator and wait until stable.
  * @retval None
  */
void xosc_init(void);

/**
  * @brief  Enable the XOSC peripheral clock via CLK_PERI_CTRL.
  * @retval None
  */
void xosc_enable_peri_clk(void);

/**
  * @brief  Switch CLK_REF source to XOSC for a stable 12 MHz clk_sys.
  * @retval None
  */
void xosc_set_clk_ref(void);

#endif /* __RP2350_XOSC_H */

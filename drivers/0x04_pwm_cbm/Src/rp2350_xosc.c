/**
  ******************************************************************************
  * @file    rp2350_xosc.c
  * @author  Kevin Thomas
  * @brief   XOSC driver implementation for RP2350.
  *
  *          Configures the external crystal oscillator and enables the
  *          peripheral clock sourced from XOSC.
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

#include "rp2350_xosc.h"

void xosc_init(void)
{
  XOSC->STARTUP = 0x00C4U;
  XOSC->CTRL = 0x00FABAA0U;
  while ((XOSC->STATUS & (1U << XOSC_STATUS_STABLE_SHIFT)) == 0) {
  }
}

void xosc_enable_peri_clk(void)
{
  uint32_t value;
  value = CLOCKS->CLK_PERI_CTRL;
  value |= (1U << CLK_PERI_CTRL_ENABLE_SHIFT);
  value |= (CLK_PERI_CTRL_AUXSRC_XOSC << CLK_PERI_CTRL_AUXSRC_SHIFT);
  CLOCKS->CLK_PERI_CTRL = value;
}

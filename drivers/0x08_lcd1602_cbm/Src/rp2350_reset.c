/**
  ******************************************************************************
  * @file    rp2350_reset.c
  * @author  Kevin Thomas
  * @brief   Reset controller driver implementation for RP2350.
  *
  *          Releases IO_BANK0 from reset and waits until the subsystem
  *          is ready.
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

#include "rp2350_reset.h"

/**
  * @brief  Release IO_BANK0 from reset and wait until ready.
  * @retval None
  */
void reset_init_subsystem(void)
{
  uint32_t value;
  value = RESETS->RESET;
  value &= ~(1U << RESETS_RESET_IO_BANK0_SHIFT);
  RESETS->RESET = value;
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_IO_BANK0_SHIFT)) == 0) {
  }
}

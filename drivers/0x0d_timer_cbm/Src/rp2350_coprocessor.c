/**
  ******************************************************************************
  * @file    rp2350_coprocessor.c
  * @author  Kevin Thomas
  * @brief   Coprocessor access control driver implementation for RP2350.
  *
  *          Grants access to coprocessors 0 and 1 by setting the
  *          corresponding bits in CPACR with DSB/ISB barriers.
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

#include "rp2350_coprocessor.h"

void coprocessor_enable(void)
{
  uint32_t value;
  value = *CPACR;
  value |= (1U << CPACR_CP1_SHIFT);
  value |= (1U << CPACR_CP0_SHIFT);
  *CPACR = value;
  __asm__ volatile ("dsb");
  __asm__ volatile ("isb");
}

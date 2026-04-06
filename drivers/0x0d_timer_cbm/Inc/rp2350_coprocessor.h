/**
  ******************************************************************************
  * @file    rp2350_coprocessor.h
  * @author  Kevin Thomas
  * @brief   Coprocessor access control driver header for RP2350.
  *
  *          Enables coprocessor access via the CPACR register.
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

#ifndef __RP2350_COPROCESSOR_H
#define __RP2350_COPROCESSOR_H

#include "rp2350.h"

/**
  * @brief  Enable coprocessor access via CPACR with DSB/ISB barriers.
  * @retval None
  */
void coprocessor_enable(void);

#endif /* __RP2350_COPROCESSOR_H */

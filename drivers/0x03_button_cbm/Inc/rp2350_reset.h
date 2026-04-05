/**
  ******************************************************************************
  * @file    rp2350_reset.h
  * @author  Kevin Thomas
  * @brief   Reset controller driver header for RP2350.
  *
  *          Provides subsystem reset release for IO_BANK0.
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

#ifndef __RP2350_RESET_H
#define __RP2350_RESET_H

#include "rp2350.h"

/**
  * @brief  Release IO_BANK0 from reset and wait until ready.
  * @retval None
  */
void reset_init_subsystem(void);

#endif /* __RP2350_RESET_H */

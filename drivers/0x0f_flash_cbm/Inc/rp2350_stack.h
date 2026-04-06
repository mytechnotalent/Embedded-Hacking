/**
  ******************************************************************************
  * @file    rp2350_stack.h
  * @author  Kevin Thomas
  * @brief   Stack pointer initialization header for RP2350.
  *
  *          Sets MSP, PSP, MSPLIM, and PSPLIM from the STACK_TOP and
  *          STACK_LIMIT values defined in rp2350.h.
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

#ifndef __RP2350_STACK_H
#define __RP2350_STACK_H

#include "rp2350.h"

/**
  * @brief  Initialize MSP, PSP, MSPLIM, and PSPLIM stack pointers.
  * @retval None
  */
void stack_init(void);

#endif /* __RP2350_STACK_H */

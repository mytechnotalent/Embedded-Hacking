/**
  ******************************************************************************
  * @file    rp2350_stack.c
  * @author  Kevin Thomas
  * @brief   Stack pointer initialization for RP2350.
  *
  *          Sets MSP, PSP, MSPLIM, and PSPLIM using inline assembly.
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

#include "rp2350_stack.h"

void stack_init(void)
{
  __asm__ volatile (
    "ldr   r0, =%0\n\t"
    "msr   PSP, r0\n\t"
    "ldr   r0, =%1\n\t"
    "msr   MSPLIM, r0\n\t"
    "msr   PSPLIM, r0\n\t"
    "ldr   r0, =%0\n\t"
    "msr   MSP, r0\n\t"
    :
    : "i" (STACK_TOP), "i" (STACK_LIMIT)
    : "r0"
  );
}

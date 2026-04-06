/**
  ******************************************************************************
  * @file    rp2350_delay.c
  * @author  Kevin Thomas
  * @brief   Delay driver implementation for RP2350.
  *
  *          Busy-wait millisecond and microsecond delays calibrated for
  *          a 12 MHz clock.
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

#include "rp2350_delay.h"

/**
  * @brief  Delay for the specified number of milliseconds.
  * @param  ms number of milliseconds to delay
  * @retval None
  */
void delay_ms(uint32_t ms)
{
  if (ms == 0)
    return;
  __asm__ volatile (
    "mov r4, #3600\n\t"
    "mul r5, %0, r4\n\t"
    "1:\n\t"
    "subs r5, #1\n\t"
    "bne 1b\n\t"
    :
    : "r" (ms)
    : "r4", "r5", "cc"
  );
}

/**
  * @brief  Delay for the specified number of microseconds.
  * @param  us number of microseconds to delay
  * @retval None
  */
void delay_us(uint32_t us)
{
  if (us == 0)
    return;
  __asm__ volatile (
    "mov r4, #4\n\t"
    "mul r5, %0, r4\n\t"
    "1:\n\t"
    "subs r5, #1\n\t"
    "bne 1b\n\t"
    :
    : "r" (us)
    : "r4", "r5", "cc"
  );
}

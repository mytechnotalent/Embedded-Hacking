/**
  ******************************************************************************
  * @file    rp2350_reset_handler.h
  * @author  Kevin Thomas
  * @brief   Reset handler header for RP2350.
  *
  *          Entry point after reset. Performs stack initialization, XOSC
  *          setup, subsystem reset release, UART initialization,
  *          and branches to main().
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

#ifndef __RP2350_RESET_HANDLER_H
#define __RP2350_RESET_HANDLER_H

/**
  * @brief  Reset handler entry point (naked, noreturn).
  * @retval None
  */
void Reset_Handler(void) __attribute__((noreturn));

#endif /* __RP2350_RESET_HANDLER_H */

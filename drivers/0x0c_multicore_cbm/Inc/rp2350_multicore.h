/**
  ******************************************************************************
  * @file    rp2350_multicore.h
  * @author  Kevin Thomas
  * @brief   Multicore driver header for RP2350.
  *
  *          Bare-metal dual-core driver using the SIO inter-processor
  *          FIFOs. Provides core 1 launch, blocking push, and blocking
  *          pop operations for 32-bit mailbox messaging.
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

#ifndef __RP2350_MULTICORE_H
#define __RP2350_MULTICORE_H

#include "rp2350.h"

/**
  * @brief  Launch a function on processor core 1.
  *
  *         Resets core 1 via the PSM, then performs the FIFO handshake
  *         protocol described in RP2350 datasheet Section 5.3 to pass
  *         the vector table, stack pointer, and entry point.
  *
  * @param  entry pointer to the void(void) function to run on core 1
  * @retval None
  */
void multicore_launch(void (*entry)(void));

/**
  * @brief  Push a 32-bit value into the inter-core FIFO (blocking).
  *
  *         Blocks until there is space in the TX FIFO, then writes the
  *         value and signals the other core with SEV.
  *
  * @param  data 32-bit value to send
  * @retval None
  */
void multicore_fifo_push(uint32_t data);

/**
  * @brief  Pop a 32-bit value from the inter-core FIFO (blocking).
  *
  *         Blocks with WFE until a value is available in the RX FIFO,
  *         then returns it.
  *
  * @retval uint32_t value received from the other core
  */
uint32_t multicore_fifo_pop(void);

#endif /* __RP2350_MULTICORE_H */

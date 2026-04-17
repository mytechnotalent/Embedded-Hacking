/**
 * @file rp2350_multicore.h
 * @brief Multicore driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Bare-metal dual-core driver using the SIO inter-processor
 * FIFOs. Provides core 1 launch, blocking push, and blocking
 * pop operations for 32-bit mailbox messaging.
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

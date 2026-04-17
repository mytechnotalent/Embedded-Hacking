/**
 * @file rp2350_multicore.c
 * @brief Multicore driver implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Implements bare-metal core 1 launch via the PSM reset and
 * SIO FIFO handshake protocol (RP2350 datasheet Section 5.3).
 * Provides blocking push/pop for inter-core 32-bit messaging.
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
#include "rp2350_multicore.h"

/**
  * @brief  Number of 32-bit words in the core 1 stack (4096 bytes).
  */
#define CORE1_STACK_WORDS      1024U

/**
  * @brief  Core 1 stack array allocated in BSS.
  */
static uint32_t core1_stack[CORE1_STACK_WORDS];

/**
  * @brief  Drain all pending values from the RX FIFO.
  * @retval None
  */
static void fifo_drain(void)
{
  while (SIO[SIO_FIFO_ST_OFFSET] & SIO_FIFO_ST_VLD_MASK)
    (void)SIO[SIO_FIFO_RD_OFFSET];
}

/**
  * @brief  Push one 32-bit word to the TX FIFO, blocking until ready.
  * @param  data value to write
  * @retval None
  */
static void fifo_push_blocking(uint32_t data)
{
  while (!(SIO[SIO_FIFO_ST_OFFSET] & SIO_FIFO_ST_RDY_MASK)) {}
  SIO[SIO_FIFO_WR_OFFSET] = data;
  __asm__ volatile ("sev");
}

/**
  * @brief  Pop one 32-bit word from the RX FIFO, blocking until valid.
  * @retval uint32_t value read from the FIFO
  */
static uint32_t fifo_pop_blocking(void)
{
  while (!(SIO[SIO_FIFO_ST_OFFSET] & SIO_FIFO_ST_VLD_MASK)) {}
  return SIO[SIO_FIFO_RD_OFFSET];
}

/**
  * @brief  Force core 1 into reset via PSM atomic set alias.
  * @retval None
  */
static void set_frce_off_proc1(void)
{
  volatile uint32_t *set = (volatile uint32_t *)((uintptr_t)&PSM->FRCE_OFF + ATOMIC_SET_OFFSET);
  *set = (1U << PSM_FRCE_OFF_PROC1_SHIFT);
  while (!(PSM->FRCE_OFF & (1U << PSM_FRCE_OFF_PROC1_SHIFT))) {}
}

/**
  * @brief  Release core 1 from reset via PSM atomic clear alias.
  * @retval None
  */
static void clr_frce_off_proc1(void)
{
  volatile uint32_t *clr = (volatile uint32_t *)((uintptr_t)&PSM->FRCE_OFF + ATOMIC_CLR_OFFSET);
  *clr = (1U << PSM_FRCE_OFF_PROC1_SHIFT);
}

/**
  * @brief  Reset core 1 and wait for its boot FIFO acknowledgement.
  * @retval None
  */
static void reset_core1(void)
{
  set_frce_off_proc1();
  clr_frce_off_proc1();
  (void)fifo_pop_blocking();
}

/**
  * @brief  Send one handshake word, draining the FIFO first for zeroes.
  * @param  cmd the command word to send
  * @retval None
  */
static void send_handshake_word(uint32_t cmd)
{
  if (!cmd) 
  {
    fifo_drain();
    __asm__ volatile ("sev");
  }
  fifo_push_blocking(cmd);
}

/**
  * @brief  Perform the six-word FIFO handshake to launch core 1.
  * @param  entry pointer to the core 1 entry function
  * @retval None
  */
static void launch_handshake(void (*entry)(void))
{
  extern uint32_t __Vectors;
  uint32_t *sp = &core1_stack[CORE1_STACK_WORDS];
  const uint32_t seq[] = {0, 0, 1, (uintptr_t)&__Vectors, (uintptr_t)sp, (uintptr_t)entry};
  uint32_t idx = 0;
  do {
    send_handshake_word(seq[idx]);
    idx = (fifo_pop_blocking() == seq[idx]) ? idx + 1 : 0;
  } while (idx < 6);
}

void multicore_launch(void (*entry)(void))
{
  reset_core1();
  launch_handshake(entry);
}

void multicore_fifo_push(uint32_t data)
{
  fifo_push_blocking(data);
}

uint32_t multicore_fifo_pop(void)
{
  return fifo_pop_blocking();
}

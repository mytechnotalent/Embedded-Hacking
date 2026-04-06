/**
  ******************************************************************************
  * @file    rp2350_multicore.c
  * @author  Kevin Thomas
  * @brief   Multicore driver implementation for RP2350.
  *
  *          Implements bare-metal core 1 launch via the PSM reset and
  *          SIO FIFO handshake protocol (RP2350 datasheet Section 5.3).
  *          Provides blocking push/pop for inter-core 32-bit messaging.
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

#include "rp2350_multicore.h"

/**
  * @brief  Number of 32-bit words in the core 1 stack (4096 bytes).
  */
#define CORE1_STACK_WORDS      1024U

/**
  * @brief  Core 1 stack array allocated in BSS.
  */
static uint32_t _core1_stack[CORE1_STACK_WORDS];

/**
  * @brief  Drain all pending values from the RX FIFO.
  * @retval None
  */
static void _fifo_drain(void)
{
  while (SIO[SIO_FIFO_ST_OFFSET] & SIO_FIFO_ST_VLD_MASK)
    (void)SIO[SIO_FIFO_RD_OFFSET];
}

/**
  * @brief  Push one 32-bit word to the TX FIFO, blocking until ready.
  * @param  data value to write
  * @retval None
  */
static void _fifo_push_blocking(uint32_t data)
{
  while (!(SIO[SIO_FIFO_ST_OFFSET] & SIO_FIFO_ST_RDY_MASK)) {
  }
  SIO[SIO_FIFO_WR_OFFSET] = data;
  __asm__ volatile ("sev");
}

/**
  * @brief  Pop one 32-bit word from the RX FIFO, blocking until valid.
  * @retval uint32_t value read from the FIFO
  */
static uint32_t _fifo_pop_blocking(void)
{
  while (!(SIO[SIO_FIFO_ST_OFFSET] & SIO_FIFO_ST_VLD_MASK)) {
  }
  return SIO[SIO_FIFO_RD_OFFSET];
}

/**
  * @brief  Force core 1 into reset via PSM atomic set alias.
  * @retval None
  */
static void _set_frce_off_proc1(void)
{
  volatile uint32_t *set = (volatile uint32_t *)((uintptr_t)&PSM->FRCE_OFF + ATOMIC_SET_OFFSET);
  *set = (1U << PSM_FRCE_OFF_PROC1_SHIFT);
  while (!(PSM->FRCE_OFF & (1U << PSM_FRCE_OFF_PROC1_SHIFT))) {
  }
}

/**
  * @brief  Release core 1 from reset via PSM atomic clear alias.
  * @retval None
  */
static void _clr_frce_off_proc1(void)
{
  volatile uint32_t *clr = (volatile uint32_t *)((uintptr_t)&PSM->FRCE_OFF + ATOMIC_CLR_OFFSET);
  *clr = (1U << PSM_FRCE_OFF_PROC1_SHIFT);
}

/**
  * @brief  Reset core 1 and wait for its boot FIFO acknowledgement.
  * @retval None
  */
static void _reset_core1(void)
{
  _set_frce_off_proc1();
  _clr_frce_off_proc1();
  (void)_fifo_pop_blocking();
}

/**
  * @brief  Send one handshake word, draining the FIFO first for zeroes.
  * @param  cmd the command word to send
  * @retval None
  */
static void _send_handshake_word(uint32_t cmd)
{
  if (!cmd) {
    _fifo_drain();
    __asm__ volatile ("sev");
  }
  _fifo_push_blocking(cmd);
}

/**
  * @brief  Perform the six-word FIFO handshake to launch core 1.
  * @param  entry pointer to the core 1 entry function
  * @retval None
  */
static void _launch_handshake(void (*entry)(void))
{
  extern uint32_t __Vectors;
  uint32_t *sp = &_core1_stack[CORE1_STACK_WORDS];
  const uint32_t seq[] = {0, 0, 1, (uintptr_t)&__Vectors, (uintptr_t)sp, (uintptr_t)entry};
  uint32_t idx = 0;
  do {
    _send_handshake_word(seq[idx]);
    idx = (_fifo_pop_blocking() == seq[idx]) ? idx + 1 : 0;
  } while (idx < 6);
}

void multicore_launch(void (*entry)(void))
{
  _reset_core1();
  _launch_handshake(entry);
}

void multicore_fifo_push(uint32_t data)
{
  _fifo_push_blocking(data);
}

uint32_t multicore_fifo_pop(void)
{
  return _fifo_pop_blocking();
}

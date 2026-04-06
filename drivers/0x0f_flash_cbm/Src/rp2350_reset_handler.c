/**
  ******************************************************************************
  * @file    rp2350_reset_handler.c
  * @author  Kevin Thomas
  * @brief   Reset handler implementation for RP2350.
  *
  *          Entry point after power-on or system reset. Copies the
  *          .data section from flash to RAM, initializes the stack,
  *          XOSC, subsystem resets, UART, then branches to main().
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

#include "rp2350_reset_handler.h"
#include "rp2350_stack.h"
#include "rp2350_xosc.h"
#include "rp2350_reset.h"
#include "rp2350_uart.h"

extern int main(void);

/**
  * @brief  Linker-defined symbol: flash LMA of .data section.
  */
extern uint32_t __data_lma;

/**
  * @brief  Linker-defined symbol: RAM VMA start of .data section.
  */
extern uint32_t __data_start;

/**
  * @brief  Linker-defined symbol: RAM VMA end of .data section.
  */
extern uint32_t __data_end;

/**
  * @brief  Copy initialized data and RAM-resident code from flash to RAM.
  * @retval None
  */
static void _data_copy_init(void)
{
  uint32_t *src = &__data_lma;
  uint32_t *dst = &__data_start;
  while (dst < &__data_end)
    *dst++ = *src++;
}

/**
  * @brief  Initialize stack pointers and copy .data section to RAM.
  * @retval None
  */
void ram_init(void)
{
  stack_init();
  _data_copy_init();
}

/**
  * @brief  Reset handler entry point (naked, noreturn).
  * @retval None
  */
void __attribute__((naked, noreturn)) Reset_Handler(void)
{
  __asm__ volatile (
    "bl ram_init\n\t"
    "bl xosc_init\n\t"
    "bl xosc_enable_peri_clk\n\t"
    "bl reset_init_subsystem\n\t"
    "bl uart_release_reset\n\t"
    "bl uart_init\n\t"
    "b main\n\t"
  );
}

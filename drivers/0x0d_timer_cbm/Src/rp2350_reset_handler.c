/**
  ******************************************************************************
  * @file    rp2350_reset_handler.c
  * @author  Kevin Thomas
  * @brief   Reset handler implementation for RP2350.
  *
  *          Entry point after power-on or system reset. Initializes the
  *          stack, XOSC, subsystem resets, UART, TIMER0 tick
  *          generator, then branches to main().
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
#include "rp2350_timer.h"

extern int main(void);

/**
  * @brief  Initialize late peripherals (timer release and tick init).
  * @retval None
  */
void _late_init(void)
{
  timer_release_reset();
  timer_tick_init();
}

/**
  * @brief  Reset handler entry point (naked, noreturn).
  * @retval None
  */
void __attribute__((naked, noreturn)) Reset_Handler(void)
{
  __asm__ volatile (
    "bl stack_init\n\t"
    "bl xosc_init\n\t"
    "bl xosc_enable_peri_clk\n\t"
    "bl reset_init_subsystem\n\t"
    "bl uart_release_reset\n\t"
    "bl uart_init\n\t"
    "bl _late_init\n\t"
    "b main\n\t"
  );
}

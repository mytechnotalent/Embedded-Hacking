/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   Repeating timer alarm demonstration.
  *
  *          Configures TIMER0 alarm 0 to fire a callback every 1 second.
  *          The callback prints "Timer heartbeat" over UART. The main
  *          loop idles with WFI.
  *
  *          Wiring:
  *            GPIO0 -> UART TX (USB-to-UART adapter RX)
  *            GPIO1 -> UART RX (USB-to-UART adapter TX)
  *            No external components required
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

#include "rp2350_timer.h"
#include "rp2350_uart.h"

/**
  * @brief  Heartbeat callback invoked by TIMER0 alarm 0 IRQ.
  * @retval None
  */
static void _heartbeat(void)
{
  uart_puts("Timer heartbeat\r\n");
}

int main(void)
{
  uart_puts("Timer alarm demo initialized\r\n");
  timer_alarm_start(1000, _heartbeat);
  while (1)
    __asm__ volatile ("wfi");
}

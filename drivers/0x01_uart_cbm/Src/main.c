/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   UART demonstration: echo received characters in uppercase.
  *
  *          Demonstrates hardware UART0 using the bare-metal UART driver.
  *          Characters typed into a terminal via a USB-to-UART adapter
  *          are echoed back in uppercase.
  *
  *          Wiring:
  *            GPIO0 (TX) -> USB-to-UART adapter RX
  *            GPIO1 (RX) -> USB-to-UART adapter TX
  *            GND        -> USB-to-UART adapter GND
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

#include "rp2350_uart.h"

int main(void)
{
  uart_puts("UART driver ready (115200 8N1)\r\n");
  uart_puts("Type characters to echo them back in UPPERCASE:\r\n");
  while (1) 
  {
    if (uart_is_readable()) 
    {
      char c = uart_getchar();
      char upper = uart_to_upper(c);
      uart_putchar(upper);
    }
  }
}

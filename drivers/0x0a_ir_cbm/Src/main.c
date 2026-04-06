/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   NEC IR receiver demonstration.
  *
  *          Decodes NEC infrared remote frames on GPIO5 and prints
  *          each command byte in hex and decimal over UART.
  *
  *          Wiring:
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO5  -> IR receiver OUT (e.g. VS1838B or TSOP4838)
  *            3.3V   -> IR receiver VCC
  *            GND    -> IR receiver GND
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

#include "rp2350_ir.h"
#include "rp2350_uart.h"
#include "rp2350_xosc.h"

/**
  * @brief  Hex digit lookup table for byte-to-hex conversion.
  */
static const char _hex_lut[16] = "0123456789ABCDEF";

/**
  * @brief  Print a byte as a two-digit hex string over UART.
  * @param  value byte to print
  * @retval None
  */
static void _print_hex(uint8_t value)
{
  char buf[3];
  buf[0] = _hex_lut[value >> 4];
  buf[1] = _hex_lut[value & 0x0FU];
  buf[2] = '\0';
  uart_puts(buf);
}

/**
  * @brief  Convert a uint8_t to a decimal string.
  * @param  value number to convert (0-255)
  * @param  buf   output buffer (at least 4 bytes)
  * @retval None
  */
static void _uint8_to_str(uint8_t value, char *buf)
{
  uint8_t idx = 0;
  if (value >= 100)
    buf[idx++] = (char)('0' + value / 100);
  if (value >= 10)
    buf[idx++] = (char)('0' + (value / 10) % 10);
  buf[idx++] = (char)('0' + value % 10);
  buf[idx] = '\0';
}

/**
  * @brief  Print a byte as a decimal string over UART.
  * @param  value byte to print (0-255)
  * @retval None
  */
static void _print_dec(uint8_t value)
{
  char buf[4];
  _uint8_to_str(value, buf);
  uart_puts(buf);
}

/**
  * @brief  Print a decoded NEC command in hex and decimal over UART.
  * @param  command decoded command byte
  * @retval None
  */
static void _print_command(uint8_t command)
{
  uart_puts("NEC command: 0x");
  _print_hex(command);
  uart_puts("  (");
  _print_dec(command);
  uart_puts(")\r\n");
}

/**
  * @brief  Initialize clocks, timer, IR receiver, and announce over UART.
  * @retval None
  */
static void _ir_setup(void)
{
  xosc_set_clk_ref();
  ir_timer_release_reset();
  ir_timer_start_tick();
  ir_init();
  uart_puts("NEC IR driver initialized on GPIO5\r\n");
  uart_puts("Press a button on your NEC remote...\r\n");
}

int main(void)
{
  int command;
  _ir_setup();
  while (1) {
    command = ir_getkey();
    if (command >= 0)
      _print_command((uint8_t)command);
  }
}

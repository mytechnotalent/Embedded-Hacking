/**
 * @file main.c
 * @brief NEC IR receiver demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Decodes NEC infrared remote frames on GPIO5 and prints
 * each command byte in hex and decimal over UART.
 *
 * Wiring:
 *  GPIO0  -> UART TX (USB-to-UART adapter RX)
 *  GPIO1  -> UART RX (USB-to-UART adapter TX)
 *  GPIO5  -> IR receiver OUT (e.g. VS1838B or TSOP4838)
 *  3.3V   -> IR receiver VCC
 *  GND    -> IR receiver GND
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
#include "rp2350_ir.h"
#include "rp2350_uart.h"
#include "rp2350_xosc.h"

/**
  * @brief  Hex digit lookup table for byte-to-hex conversion.
  */
static const char hex_lut[16] = "0123456789ABCDEF";

/**
  * @brief  Print a byte as a two-digit hex string over UART.
  * @param  value byte to print
  * @retval None
  */
static void print_hex(uint8_t value)
{
  char buf[3];
  buf[0] = hex_lut[value >> 4];
  buf[1] = hex_lut[value & 0x0FU];
  buf[2] = '\0';
  uart_puts(buf);
}

/**
  * @brief  Convert a uint8_t to a decimal string.
  * @param  value number to convert (0-255)
  * @param  buf   output buffer (at least 4 bytes)
  * @retval None
  */
static void uint8_to_str(uint8_t value, char *buf)
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
static void print_dec(uint8_t value)
{
  char buf[4];
  uint8_to_str(value, buf);
  uart_puts(buf);
}

/**
  * @brief  Print a decoded NEC command in hex and decimal over UART.
  * @param  command decoded command byte
  * @retval None
  */
static void print_command(uint8_t command)
{
  uart_puts("NEC command: 0x");
  print_hex(command);
  uart_puts("  (");
  print_dec(command);
  uart_puts(")\r\n");
}

/**
  * @brief  Initialize clocks, timer, IR receiver, and announce over UART.
  * @retval None
  */
static void ir_setup(void)
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
  ir_setup();
  while (1) 
  {
    command = ir_getkey();
    if (command >= 0)
      print_command((uint8_t)command);
  }
}

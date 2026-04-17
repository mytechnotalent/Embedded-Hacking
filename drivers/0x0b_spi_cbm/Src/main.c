/**
 * @file main.c
 * @brief SPI loopback demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Performs a full-duplex SPI0 transfer in master mode with
 * MOSI wired to MISO for loopback verification. Prints TX
 * and RX data over UART every second.
 *
 * Wiring (loopback test):
 *  GPIO0  -> UART TX (USB-to-UART adapter RX)
 *  GPIO1  -> UART RX (USB-to-UART adapter TX)
 *  GPIO19 (MOSI) -> GPIO16 (MISO)
 *  GPIO18 (SCK)  -> logic analyzer (optional)
 *  GPIO17 (CS)   -> active-low slave (optional)
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
#include "rp2350_spi.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"
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
  * @brief  Print a buffer as hex bytes separated by spaces over UART.
  * @param  label text label to print before the data
  * @param  buf   byte buffer to print
  * @param  len   number of bytes in buffer
  * @retval None
  */
static void print_buffer(const char *label, const uint8_t *buf, uint32_t len)
{
  uart_puts(label);
  for (uint32_t i = 0; i < len; i++) 
  {
    print_hex(buf[i]);
    if (i + 1 < len)
      uart_putchar(' ');
  }
  uart_puts("\r\n");
}

/**
  * @brief  Perform one SPI loopback transfer and print TX/RX over UART.
  * @param  tx_buf transmit buffer
  * @param  rx_buf receive buffer (cleared after printing)
  * @param  len    number of bytes to transfer
  * @retval None
  */
static void loopback_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len)
{
  spi_cs_select();
  spi_transfer(tx_buf, rx_buf, len);
  spi_cs_deselect();
  print_buffer("TX: ", tx_buf, len);
  print_buffer("RX: ", rx_buf, len);
  uart_puts("\r\n");
}

/**
  * @brief  Clear a byte buffer to zero.
  * @param  buf pointer to buffer
  * @param  len number of bytes to clear
  * @retval None
  */
static void clear_buffer(uint8_t *buf, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++)
    buf[i] = 0;
}

/**
  * @brief  Initialize clocks, SPI peripheral, and announce over UART.
  * @retval None
  */
static void spi_setup(void)
{
  xosc_set_clk_ref();
  spi_release_reset();
  spi_init();
  uart_puts("SPI loopback initialized (MOSI->MISO on GPIO19->GPIO16)\r\n");
}

int main(void)
{
  /** @brief  Transmit test pattern for SPI loopback. */
  static const uint8_t tx[] = {0xDE, 0xAD, 0xBE, 0xEF};
  /** @brief  Buffer length for SPI loopback transfer. */
  static const uint32_t len = sizeof(tx);
  uint8_t rx[sizeof(tx)] = {0};
  spi_setup();
  while (1) 
  {
    loopback_transfer(tx, rx, len);
    clear_buffer(rx, len);
    delay_ms(1000);
  }
}

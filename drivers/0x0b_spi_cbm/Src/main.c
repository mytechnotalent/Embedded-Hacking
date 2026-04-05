/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   SPI loopback demonstration.
  *
  *          Performs a full-duplex SPI0 transfer in master mode with
  *          MOSI wired to MISO for loopback verification. Prints TX
  *          and RX data over UART every second.
  *
  *          Wiring (loopback test):
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO19 (MOSI) -> GPIO16 (MISO)
  *            GPIO18 (SCK)  -> logic analyzer (optional)
  *            GPIO17 (CS)   -> active-low slave (optional)
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

#include "rp2350_spi.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"
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
  * @brief  Print a buffer as hex bytes separated by spaces over UART.
  * @param  label text label to print before the data
  * @param  buf   byte buffer to print
  * @param  len   number of bytes in buffer
  * @retval None
  */
static void _print_buffer(const char *label, const uint8_t *buf, uint32_t len)
{
  uart_puts(label);
  for (uint32_t i = 0; i < len; i++) {
    _print_hex(buf[i]);
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
static void _loopback_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len)
{
  spi_cs_select();
  spi_transfer(tx_buf, rx_buf, len);
  spi_cs_deselect();
  _print_buffer("TX: ", tx_buf, len);
  _print_buffer("RX: ", rx_buf, len);
  uart_puts("\r\n");
}

/**
  * @brief  Clear a byte buffer to zero.
  * @param  buf pointer to buffer
  * @param  len number of bytes to clear
  * @retval None
  */
static void _clear_buffer(uint8_t *buf, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++)
    buf[i] = 0;
}

/**
  * @brief  Application entry point for the SPI loopback demo.
  * @retval int does not return
  */
int main(void)
{
  /** @brief  Transmit test pattern for SPI loopback. */
  static const uint8_t tx[] = {0xDE, 0xAD, 0xBE, 0xEF};
  /** @brief  Buffer length for SPI loopback transfer. */
  static const uint32_t len = sizeof(tx);
  uint8_t rx[sizeof(tx)] = {0};
  xosc_set_clk_ref();
  spi_release_reset();
  spi_init();
  uart_puts("SPI loopback initialized (MOSI->MISO on GPIO19->GPIO16)\r\n");
  while (1) {
    _loopback_transfer(tx, rx, len);
    _clear_buffer(rx, len);
    delay_ms(1000);
  }
}

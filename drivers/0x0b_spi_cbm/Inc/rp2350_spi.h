/**
 * @file rp2350_spi.h
 * @brief SPI0 master driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Provides full-duplex SPI0 master mode on GPIO16-19 with
 * software-controlled chip select. Configures Motorola SPI
 * frame format, 8-bit data, CPOL=0, CPHA=0, 1 MHz clock.
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
#ifndef __RP2350_SPI_H
#define __RP2350_SPI_H

#include "rp2350.h"

/**
  * @brief  Release SPI0 from reset in the RESETS controller.
  * @retval None
  */
void spi_release_reset(void);

/**
  * @brief  Initialise SPI0 in master mode at 1 MHz, 8-bit, CPOL=0/CPHA=0.
  *
  *         Configures SSPCR0, SSPCPSR, and SSPCR1 then enables the port.
  *         Also configures GPIO16-19 pads and IO funcsel for SPI.
  *
  * @retval None
  */
void spi_init(void);

/**
  * @brief  Assert the chip-select line (drive CS low).
  * @retval None
  */
void spi_cs_select(void);

/**
  * @brief  Deassert the chip-select line (drive CS high).
  * @retval None
  */
void spi_cs_deselect(void);

/**
  * @brief  Perform a full-duplex SPI transfer.
  *
  *         Sends @p len bytes from @p tx while simultaneously receiving
  *         @p len bytes into @p rx. The caller is responsible for
  *         asserting and deasserting CS around this call.
  *
  * @param  tx  pointer to transmit buffer (must be @p len bytes)
  * @param  rx  pointer to receive buffer  (must be @p len bytes)
  * @param  len number of bytes to transfer
  * @retval None
  */
void spi_transfer(const uint8_t *tx, uint8_t *rx, uint32_t len);

#endif /* __RP2350_SPI_H */

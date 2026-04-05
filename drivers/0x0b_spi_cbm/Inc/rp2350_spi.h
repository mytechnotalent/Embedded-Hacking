/**
  ******************************************************************************
  * @file    rp2350_spi.h
  * @author  Kevin Thomas
  * @brief   SPI0 master driver header for RP2350.
  *
  *          Provides full-duplex SPI0 master mode on GPIO16-19 with
  *          software-controlled chip select. Configures Motorola SPI
  *          frame format, 8-bit data, CPOL=0, CPHA=0, 1 MHz clock.
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

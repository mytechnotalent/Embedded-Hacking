/**
  ******************************************************************************
  * @file    rp2350_spi.c
  * @author  Kevin Thomas
  * @brief   SPI0 master driver implementation for RP2350.
  *
  *          Full-duplex SPI0 master on GPIO16 (MISO), GPIO17 (CS),
  *          GPIO18 (SCK), GPIO19 (MOSI). Motorola SPI frame format,
  *          8-bit data, CPOL=0, CPHA=0, 1 MHz clock from 12 MHz
  *          clk_peri.
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

/**
  * @brief  Bit mask for the chip-select pin.
  */
#define CS_PIN_MASK  (1U << SPI_CS_PIN)

/**
  * @brief  Clear the SPI0 reset bit in the reset controller.
  * @retval None
  */
static void _spi_clear_reset(void)
{
  uint32_t value;
  value = RESETS->RESET;
  value &= ~(1U << RESETS_RESET_SPI0_SHIFT);
  RESETS->RESET = value;
}

/**
  * @brief  Wait until SPI0 is out of reset.
  * @retval None
  */
static void _spi_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_SPI0_SHIFT)) == 0) {
  }
}

/**
  * @brief  Configure a GPIO pad for SPI: input enabled, no pull, no isolation.
  * @param  pin GPIO pin number to configure
  * @retval None
  */
static void _configure_spi_pad(uint8_t pin)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[pin];
  value &= ~(1U << PADS_BANK0_OD_SHIFT);
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  PADS_BANK0->GPIO[pin] = value;
}

/**
  * @brief  Configure a GPIO pad for CS: output enabled, pull-up, no isolation.
  * @param  pin GPIO pin number to configure
  * @retval None
  */
static void _configure_cs_pad(uint8_t pin)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[pin];
  value &= ~(1U << PADS_BANK0_OD_SHIFT);
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value |= (1U << PADS_BANK0_PUE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  PADS_BANK0->GPIO[pin] = value;
}

/**
  * @brief  Set a GPIO funcsel to SPI alternate function.
  * @param  pin GPIO pin number
  * @retval None
  */
static void _set_funcsel_spi(uint8_t pin)
{
  IO_BANK0->GPIO[pin].CTRL = IO_BANK0_CTRL_FUNCSEL_SPI;
}

/**
  * @brief  Set a GPIO funcsel to SIO for software control (CS pin).
  * @param  pin GPIO pin number
  * @retval None
  */
static void _set_funcsel_sio(uint8_t pin)
{
  IO_BANK0->GPIO[pin].CTRL = IO_BANK0_CTRL_FUNCSEL_SIO;
}

/**
  * @brief  Configure CS pin as output, initially deasserted (high).
  * @retval None
  */
static void _cs_init(void)
{
  SIO[SIO_GPIO_OUT_SET_OFFSET] = CS_PIN_MASK;
  SIO[SIO_GPIO_OE_SET_OFFSET] = CS_PIN_MASK;
}

/**
  * @brief  Configure SSPCR0 for 8-bit Motorola SPI, SCR=0.
  * @retval None
  */
static void _configure_cr0(void)
{
  uint32_t value = 0;
  value |= (SPI_SSPCR0_DSS_8BIT << SPI_SSPCR0_DSS_SHIFT);
  value |= (SPI_SCR_1MHZ << SPI_SSPCR0_SCR_SHIFT);
  SPI0->SSPCR0 = value;
}

/**
  * @brief  Configure SSPCPSR clock prescaler for 1 MHz.
  * @retval None
  */
static void _configure_prescaler(void)
{
  SPI0->SSPCPSR = SPI_CPSDVSR_1MHZ;
}

/**
  * @brief  Enable the SPI0 peripheral (SSE=1 in SSPCR1).
  * @retval None
  */
static void _enable_spi(void)
{
  SPI0->SSPCR1 = (1U << SPI_SSPCR1_SSE_SHIFT);
}

/**
  * @brief  Wait until the SPI transmit FIFO has space.
  * @retval None
  */
static void _wait_tx_not_full(void)
{
  while ((SPI0->SSPSR & SPI_SSPSR_TNF_MASK) == 0) {
  }
}

/**
  * @brief  Wait until the SPI receive FIFO has data.
  * @retval None
  */
static void _wait_rx_not_empty(void)
{
  while ((SPI0->SSPSR & SPI_SSPSR_RNE_MASK) == 0) {
  }
}

void spi_release_reset(void)
{
  _spi_clear_reset();
  _spi_wait_reset_done();
}

/**
  * @brief  Configure all SPI and CS GPIO pads.
  * @retval None
  */
static void _configure_all_pads(void)
{
  _configure_spi_pad(SPI_MOSI_PIN);
  _configure_spi_pad(SPI_MISO_PIN);
  _configure_spi_pad(SPI_SCK_PIN);
  _configure_cs_pad(SPI_CS_PIN);
}

/**
  * @brief  Assign SPI and SIO alternate functions to all GPIO pins.
  * @retval None
  */
static void _configure_all_funcsel(void)
{
  _set_funcsel_spi(SPI_MOSI_PIN);
  _set_funcsel_spi(SPI_MISO_PIN);
  _set_funcsel_spi(SPI_SCK_PIN);
  _set_funcsel_sio(SPI_CS_PIN);
}

void spi_init(void)
{
  _configure_all_pads();
  _configure_all_funcsel();
  _cs_init();
  _configure_cr0();
  _configure_prescaler();
  _enable_spi();
}

void spi_cs_select(void)
{
  SIO[SIO_GPIO_OUT_CLR_OFFSET] = CS_PIN_MASK;
}

void spi_cs_deselect(void)
{
  SIO[SIO_GPIO_OUT_SET_OFFSET] = CS_PIN_MASK;
}

void spi_transfer(const uint8_t *tx, uint8_t *rx, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++) {
    _wait_tx_not_full();
    SPI0->SSPDR = tx[i];
    _wait_rx_not_empty();
    rx[i] = (uint8_t)SPI0->SSPDR;
  }
}

/**
  ******************************************************************************
  * @file    rp2350_i2c.h
  * @author  Kevin Thomas
  * @brief   I2C driver header for RP2350.
  *
  *          I2C1 master-mode driver at 100 kHz on SDA=GPIO2 / SCL=GPIO3.
  *          Provides init, probe, read, write, and bus scan functions.
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

#ifndef __RP2350_I2C_H
#define __RP2350_I2C_H

#include "rp2350.h"

/**
  * @brief  Release I2C1 from reset and wait for completion.
  * @retval None
  */
void i2c_release_reset(void);

/**
  * @brief  Initialize I2C1 as a 100 kHz master on SDA=GPIO2 / SCL=GPIO3.
  *
  *         Configures GPIO pads with pull-ups, sets FUNCSEL to I2C,
  *         programs SCL timing for 100 kHz at 12 MHz clk_sys, and
  *         enables the controller in master mode with 7-bit addressing.
  *
  * @retval None
  */
void i2c_init(void);

/**
  * @brief  Probe a 7-bit I2C address and return whether a device responds.
  *
  *         Sends a 1-byte read to the target address. Returns true if
  *         the device acknowledges, false otherwise.
  *
  * @param  addr 7-bit I2C address (0x08-0x77)
  * @retval bool true if a device acknowledged, false otherwise
  */
bool i2c_probe(uint8_t addr);

/**
  * @brief  Scan all valid 7-bit addresses and print a formatted table.
  *
  *         Iterates addresses 0x00 through 0x7F, probes each valid one
  *         via i2c_probe(), and prints a 16-column hex grid showing
  *         discovered device addresses over UART.
  *
  * @retval None
  */
void i2c_scan(void);

#endif /* __RP2350_I2C_H */

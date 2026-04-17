/**
 * @file rp2350_i2c.h
 * @brief I2C driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * I2C1 master-mode driver at 100 kHz on SDA=GPIO2 / SCL=GPIO3.
 * Provides init, target selection, and single-byte write.
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
  * @brief  Set the I2C1 target slave address.
  *
  *         Disables the controller, writes the 7-bit address into the
  *         TAR register, and re-enables the controller.
  *
  * @param  addr 7-bit I2C address
  * @retval None
  */
void i2c_set_target(uint8_t addr);

/**
  * @brief  Write one byte to the current target address with a STOP.
  *
  *         Sends a single data byte over I2C1 with the STOP condition.
  *         Blocks until the transfer completes or an abort is detected.
  *
  * @param  data byte to transmit
  * @retval None
  */
void i2c_write_byte(uint8_t data);

#endif /* __RP2350_I2C_H */

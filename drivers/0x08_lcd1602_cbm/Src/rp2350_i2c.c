/**
  ******************************************************************************
  * @file    rp2350_i2c.c
  * @author  Kevin Thomas
  * @brief   RP2350 I2C1 master-mode driver implementation.
  *
  *          Bare-metal driver for the RP2350 Synopsys DW APB I2C controller.
  *          Configures I2C1 at 100 kHz on SDA=GPIO2 / SCL=GPIO3 with
  *          internal pull-ups. Provides target selection and single-byte
  *          write. All register accesses verified against the RP2350
  *          datasheet (RP-008373-DS-2).
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

#include "rp2350_i2c.h"

/**
  * @brief  Configure GPIO2 and GPIO3 pads for I2C (input enabled, pull-up).
  *
  *         Clears pad isolation, enables input, enables pull-up, and sets
  *         drive strength to 4 mA on both SDA and SCL pins.
  *
  * @retval None
  */
static void _i2c_config_pads(void)
{
  uint32_t pad_val = (1U << PADS_BANK0_IE_SHIFT) | (1U << PADS_BANK0_PUE_SHIFT) | (1U << 4);
  PADS_BANK0->GPIO[I2C_SDA_PIN] = pad_val;
  PADS_BANK0->GPIO[I2C_SCL_PIN] = pad_val;
}

/**
  * @brief  Set GPIO2 and GPIO3 IO mux function to I2C (FUNCSEL=3).
  * @retval None
  */
static void _i2c_config_gpio(void)
{
  IO_BANK0->GPIO[I2C_SDA_PIN].CTRL = IO_BANK0_CTRL_FUNCSEL_I2C;
  IO_BANK0->GPIO[I2C_SCL_PIN].CTRL = IO_BANK0_CTRL_FUNCSEL_I2C;
}

/**
  * @brief  Configure I2C1 as fast-mode master with 7-bit addressing.
  *
  *         Sets MASTER_MODE, SPEED=FAST, IC_SLAVE_DISABLE, IC_RESTART_EN,
  *         and TX_EMPTY_CTRL bits in the CON register.
  *
  * @retval None
  */
static void _i2c_config_con(void)
{
  I2C1->CON = (1U << I2C_CON_MASTER_MODE_SHIFT)
            | (I2C_CON_SPEED_FAST << I2C_CON_SPEED_SHIFT)
            | (1U << I2C_CON_IC_RESTART_EN_SHIFT)
            | (1U << I2C_CON_IC_SLAVE_DISABLE_SHIFT)
            | (1U << I2C_CON_TX_EMPTY_CTRL_SHIFT);
}

/**
  * @brief  Program SCL timing for 100 kHz at 12 MHz clk_sys.
  *
  *         Sets fast-mode SCL high/low counts, SDA hold time, and
  *         spike suppression length.
  *
  * @retval None
  */
static void _i2c_config_timing(void)
{
  I2C1->FS_SCL_HCNT = I2C_FS_SCL_HCNT_VAL;
  I2C1->FS_SCL_LCNT = I2C_FS_SCL_LCNT_VAL;
  I2C1->FS_SPKLEN = I2C_FS_SPKLEN_VAL;
  I2C1->SDA_HOLD = I2C_SDA_TX_HOLD_VAL;
}

/**
  * @brief  Check for TX abort and clear if detected.
  * @retval bool true if TX abort occurred
  */
static bool _check_abort(void)
{
  if (I2C1->RAW_INTR_STAT & I2C_RAW_INTR_TX_ABRT) 
  {
    (void)I2C1->CLR_TX_ABRT;
    return true;
  }
  return false;
}

/**
  * @brief  Check for stop condition detected and clear if so.
  * @retval bool true if stop detected
  */
static bool _check_stop(void)
{
  if (I2C1->RAW_INTR_STAT & I2C_RAW_INTR_STOP_DET) 
  {
    (void)I2C1->CLR_STOP_DET;
    return true;
  }
  return false;
}

/**
  * @brief  Wait for a STOP_DET or TX_ABRT interrupt, then clear it.
  * @retval None
  */
static void _i2c_wait_done(void)
{
  uint32_t timeout = I2C_TIMEOUT;
  while (timeout > 0U) 
  {
    if (_check_abort() || _check_stop())
      break;
    timeout--;
  }
}

void i2c_release_reset(void)
{
  RESETS->RESET |= (1U << RESETS_RESET_I2C1_SHIFT);
  RESETS->RESET &= ~(1U << RESETS_RESET_I2C1_SHIFT);
  while (!(RESETS->RESET_DONE & (1U << RESETS_RESET_I2C1_SHIFT))) {}
}

void i2c_init(void)
{
  _i2c_config_pads();
  _i2c_config_gpio();
  I2C1->ENABLE = 0U;
  _i2c_config_con();
  I2C1->TX_TL = 0U;
  I2C1->RX_TL = 0U;
  _i2c_config_timing();
  I2C1->ENABLE = 1U;
}

void i2c_set_target(uint8_t addr)
{
  I2C1->ENABLE = 0U;
  I2C1->TAR = addr;
  I2C1->ENABLE = 1U;
}

void i2c_write_byte(uint8_t data)
{
  (void)I2C1->CLR_TX_ABRT;
  I2C1->DATA_CMD = data | (1U << I2C_DATA_CMD_STOP_SHIFT);
  _i2c_wait_done();
}

/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   I2C demonstration: scan all 7-bit addresses and report devices.
  *
  *          Demonstrates I2C bus scanning using the bare-metal I2C driver.
  *          I2C1 is configured at 100 kHz on SDA=GPIO2 / SCL=GPIO3. A
  *          formatted hex table of all responding device addresses is
  *          printed over UART and repeated every 5 seconds.
  *
  *          Wiring:
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO2  -> I2C device SDA (4.7 kohm pull-up to 3.3 V)
  *            GPIO3  -> I2C device SCL (4.7 kohm pull-up to 3.3 V)
  *            3.3V   -> I2C device VCC
  *            GND    -> I2C device GND
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
#include "rp2350_uart.h"
#include "rp2350_xosc.h"
#include "rp2350_delay.h"

#define SCAN_DELAY_MS 5000

int main(void)
{
  xosc_set_clk_ref();
  i2c_release_reset();
  i2c_init();
  uart_puts("I2C driver initialized: I2C1 @ 100 kHz  SDA=GPIO2  SCL=GPIO3\r\n");
  while (1) {
    i2c_scan();
    delay_ms(SCAN_DELAY_MS);
  }
}

/**
 * @file main.c
 * @brief I2C demonstration: scan all 7-bit addresses and report devices.
 * @author Kevin Thomas
 * @date 2026
 *
 * Demonstrates I2C bus scanning using the bare-metal I2C driver.
 * I2C1 is configured at 100 kHz on SDA=GPIO2 / SCL=GPIO3. A
 * formatted hex table of all responding device addresses is
 * printed over UART and repeated every 5 seconds.
 *
 * Wiring:
 *  GPIO0  -> UART TX (USB-to-UART adapter RX)
 *  GPIO1  -> UART RX (USB-to-UART adapter TX)
 *  GPIO2  -> I2C device SDA (4.7 kohm pull-up to 3.3 V)
 *  GPIO3  -> I2C device SCL (4.7 kohm pull-up to 3.3 V)
 *  3.3V   -> I2C device VCC
 *  GND    -> I2C device GND
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
#include "rp2350_i2c.h"
#include "rp2350_uart.h"
#include "rp2350_xosc.h"
#include "rp2350_delay.h"

/** @brief Delay between I2C bus scans in milliseconds */
#define SCAN_DELAY_MS 5000

int main(void)
{
  xosc_set_clk_ref();
  i2c_release_reset();
  i2c_init();
  uart_puts("I2C driver initialized: I2C1 @ 100 kHz  SDA=GPIO2  SCL=GPIO3\r\n");
  while (1) 
  {
    i2c_scan();
    delay_ms(SCAN_DELAY_MS);
  }
}

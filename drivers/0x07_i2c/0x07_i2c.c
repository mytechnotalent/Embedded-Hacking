/**
 * @file 0x07_i2c.c
 * @brief I2C demonstration: scan all 7-bit addresses and report devices
 * @author Kevin Thomas
 * @date 2025
 *
 * MIT License
 *
 * Copyright (c) 2025 Kevin Thomas
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
 *
 * -----------------------------------------------------------------------------
 *
 * Demonstrates I2C bus scanning using the i2c driver (i2c.h / i2c.c). I2C1
 * is configured at 100 kHz on SDA=GPIO2 / SCL=GPIO3. A formatted hex table
 * of all responding device addresses is printed over UART and repeated
 * every 5 seconds.
 *
 * Wiring:
 *   GPIO2 (SDA) -> I2C device SDA  (4.7 kohm pull-up to 3.3 V recommended)
 *   GPIO3 (SCL) -> I2C device SCL  (4.7 kohm pull-up to 3.3 V recommended)
 *   3.3V        -> I2C device VCC
 *   GND         -> I2C device GND
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "i2c.h"

#define I2C_PORT    1
#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3
#define I2C_BAUD    100000

int main(void) {
    stdio_init_all();
    i2c_driver_init(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_BAUD);

    printf("I2C driver initialized: I2C%d @ %d Hz  SDA=GPIO%d  SCL=GPIO%d\r\n",
           I2C_PORT, I2C_BAUD, I2C_SDA_PIN, I2C_SCL_PIN);

    while (true) {
        i2c_driver_scan(I2C_PORT);
        sleep_ms(5000);
    }
}

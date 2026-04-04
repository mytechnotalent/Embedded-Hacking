/**
 * @file gpio.h
 * @brief GPIO configuration, set, clear, toggle, and read functions
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
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Configure a GPIO pin as SIO output
 *
 * Configures the pad control (clear OD, set IE, clear ISO), sets
 * IO_BANK0 FUNCSEL to SIO (0x05), and enables the output driver
 * via the SIO output-enable set register.
 *
 * @param gpio_num GPIO pin number (0-29)
 */
void GPIO_Config(uint32_t gpio_num);

/**
 * @brief Drive a GPIO output high
 *
 * Sets the specified bit in the SIO GPIO output set register.
 *
 * @param gpio_num GPIO pin number (0-29)
 */
void GPIO_Set(uint32_t gpio_num);

/**
 * @brief Drive a GPIO output low
 *
 * Sets the specified bit in the SIO GPIO output clear register.
 *
 * @param gpio_num GPIO pin number (0-29)
 */
void GPIO_Clear(uint32_t gpio_num);

/**
 * @brief Toggle a GPIO output
 *
 * Sets the specified bit in the SIO GPIO output XOR register,
 * which inverts the current output level.
 *
 * @param gpio_num GPIO pin number (0-29)
 */
void GPIO_Toggle(uint32_t gpio_num);

/**
 * @brief Read the current input level of a GPIO pin
 *
 * Reads the SIO GPIO input register and returns the state of the
 * specified pin.
 *
 * @param gpio_num GPIO pin number (0-29)
 * @return bool true if pin is high, false if low
 */
bool GPIO_Get(uint32_t gpio_num);

#endif // GPIO_H

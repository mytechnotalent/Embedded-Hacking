/**
 * @file gpio.c
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

#include "gpio.h"
#include "constants.h"

#define SIO ((volatile uint32_t *)SIO_BASE)

/**
 * @brief Configure pad control for a GPIO pin
 *
 * @details Clears OD (output disable), sets IE (input enable), and
 *          clears ISO (isolation) in PADS_BANK0.
 *
 * @param gpio_num GPIO pin number (0-29)
 */
static void _gpio_config_pad(uint32_t gpio_num) {
    uint32_t value;

    // read PAD value
    value = PADS_BANK0->GPIO[gpio_num];
    // clear OD bit (output disable)
    value &= ~(1U << PADS_BANK0_OD_SHIFT);
    // set IE bit (input enable)
    value |= (1U << PADS_BANK0_IE_SHIFT);
    // clear ISO bit (isolate)
    value &= ~(1U << PADS_BANK0_ISO_SHIFT);
    // store value into PAD
    PADS_BANK0->GPIO[gpio_num] = value;
}

/**
 * @brief Set IO_BANK0 FUNCSEL to SIO for a GPIO pin
 *
 * @details Reads the control register, clears FUNCSEL bits, sets
 *          FUNCSEL to SIO (0x05), and writes back.
 *
 * @param gpio_num GPIO pin number (0-29)
 */
static void _gpio_config_funcsel(uint32_t gpio_num) {
    uint32_t value;

    // read CTRL value
    value = IO_BANK0->GPIO[gpio_num].CTRL;
    // clear FUNCSEL bits
    value &= ~IO_BANK0_CTRL_FUNCSEL_MASK;
    // set FUNCSEL to SIO (0x05)
    value |= IO_BANK0_CTRL_FUNCSEL_SIO;
    // store value into CTRL
    IO_BANK0->GPIO[gpio_num].CTRL = value;
}

/**
 * @brief Enable the output driver for a GPIO pin via SIO
 *
 * @details Sets the corresponding bit in the SIO output-enable set
 *          register.
 *
 * @param gpio_num GPIO pin number (0-29)
 */
static void _gpio_enable_output(uint32_t gpio_num) {
    SIO[SIO_GPIO_OE_SET_OFFSET] = (1U << gpio_num);
}

void GPIO_Config(uint32_t gpio_num) {
    _gpio_config_pad(gpio_num);
    _gpio_config_funcsel(gpio_num);
    _gpio_enable_output(gpio_num);
}

void GPIO_Set(uint32_t gpio_num) {
    SIO[SIO_GPIO_OUT_SET_OFFSET] = (1U << gpio_num);
}

void GPIO_Clear(uint32_t gpio_num) {
    SIO[SIO_GPIO_OUT_CLR_OFFSET] = (1U << gpio_num);
}

void GPIO_Toggle(uint32_t gpio_num) {
    SIO[SIO_GPIO_OUT_XOR_OFFSET] = (1U << gpio_num);
}

bool GPIO_Get(uint32_t gpio_num) {
    return (SIO[SIO_GPIO_IN_OFFSET] & (1U << gpio_num)) != 0;
}

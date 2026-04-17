/**
 * @file 0x0008_unitialized-variables-c.c
 * @brief Blink LED using direct register-level pad and IO bank configuration
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
 * Blinks an LED on GPIO16 using direct register writes to PADS_BANK0 and
 * IO_BANK0 for pad configuration, plus gpioc coprocessor bit-level functions
 * for output enable and data. Demonstrates the register-level equivalent of
 * gpio_init / gpio_set_function.
 *
 * Wiring:
 *   GPIO16 -> LED anode (with current-limiting resistor to GND)
 */

#include <stdio.h>
#include "pico/stdlib.h"

/** @brief GPIO pin number for the LED */
#define LED_PIN 16

/**
 * @brief Configure pad and IO bank registers for GPIO16 as SIO output
 *
 * @details Sets PADS_BANK0 IE/OD bits, assigns FUNCSEL to SIO in IO_BANK0,
 *          clears the ISO bit, and enables output via the coprocessor.
 *
 * @retval None
 */
static void configure_pad_and_iobank(void) {
    gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
    gpioc_bit_out_put(LED_PIN, 0);
    hw_write_masked(&pads_bank0_hw->io[LED_PIN],
                   PADS_BANK0_GPIO0_IE_BITS,
                   PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
    io_bank0_hw->io[LED_PIN].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[LED_PIN], PADS_BANK0_GPIO0_ISO_BITS);
    gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
}

/**
 * @brief Blink LED using coprocessor bit output functions
 *
 * @details Toggles the LED on and off with 500ms delays using
 *          gpioc_bit_out_put and sleep_us.
 *
 * @retval None
 */
static void blink_cycle(void) {
    gpioc_bit_out_put(LED_PIN, 1);
    sleep_us(500 * 1000ull);
    gpioc_bit_out_put(LED_PIN, 0);
    sleep_us(500 * 1000ull);
}

int main(void) {
    configure_pad_and_iobank();
    while (true) {
        blink_cycle();
    }
}

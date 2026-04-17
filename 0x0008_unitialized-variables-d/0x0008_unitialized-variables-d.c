/**
 * @file 0x0008_unitialized-variables-d.c
 * @brief Blink LED using pico_default_asm_volatile coprocessor instructions
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
 * Blinks an LED on GPIO16 using pico_default_asm_volatile to emit mcrr
 * coprocessor instructions for GPIO output enable and data, plus inline
 * assembly for pad configuration (hw_write_masked, IO_BANK0 FUNCSEL,
 * hw_clear_bits equivalents).
 *
 * Wiring:
 *   GPIO16 -> LED anode (with current-limiting resistor to GND)
 */

#include <stdio.h>
#include "pico/stdlib.h"

/** @brief GPIO pin number for the LED */
#define LED_PIN 16

/**
 * @brief Enable output and set initial coprocessor OE/OUT via mcrr
 *
 * @details Issues two mcrr p0 instructions: one to enable output direction,
 *          one to set the initial output state.
 *
 * @retval None
 */
static void asm_set_oe_and_out(void) {
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c4" : : "r" (LED_PIN), "r" (GPIO_OUT));
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c4" : : "r" (LED_PIN), "r" (GPIO_OUT));
}

/**
 * @brief Configure PADS_BANK0 IE/OD bits via inline assembly hw_xor_bits
 *
 * @details Performs a read-modify-write on the pad register to set IE
 *          and clear OD, replicating hw_write_masked behavior.
 *
 * @retval None
 */
static void asm_configure_pad(void) {
    pico_default_asm_volatile (
        "ldr r2, [%0]\n"                    // load current pad register
        "eor r2, r2, %1\n"                  // xor with IE bit
        "and r2, r2, %2\n"                  // mask with (IE|OD)
        "eor r2, r2, %1\n"                  // recombine (hw_xor_bits logic)
        "str r2, [%0]\n"                    // write back
        :
        : "r" (&pads_bank0_hw->io[LED_PIN]),
        "r" (PADS_BANK0_GPIO0_IE_BITS),
        "r" (PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS)
        : "r2", "memory"
    );
}

/**
 * @brief Set IO_BANK0 FUNCSEL to SIO via inline assembly store
 *
 * @details Writes the SIO function select value directly to the
 *          IO_BANK0 GPIO control register.
 *
 * @retval None
 */
static void asm_set_funcsel(void) {
    pico_default_asm_volatile (
        "str %1, [%0]\n"
        :
        : "r" (&io_bank0_hw->io[LED_PIN].ctrl),
        "r" (GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB)
        : "memory"
    );
}

/**
 * @brief Clear ISO bits in PADS_BANK0 via inline assembly
 *
 * @details Performs a read-modify-write to clear the pad isolation
 *          bit, un-isolating the pad for normal operation.
 *
 * @retval None
 */
static void asm_clear_iso(void) {
    pico_default_asm_volatile (
        "ldr r2, [%0]\n"                    // load current register value
        "bic r2, r2, %1\n"                  // clear the ISO bits (bit clear)
        "str r2, [%0]\n"                    // write back
        :
        : "r" (&pads_bank0_hw->io[LED_PIN]),
        "r" (PADS_BANK0_GPIO0_ISO_BITS)
        : "r2", "memory"
    );
}

/**
 * @brief Blink LED using coprocessor mcrr output instructions
 *
 * @details Toggles the LED on and off with 500ms delays using
 *          pico_default_asm_volatile mcrr instructions.
 *
 * @retval None
 */
static void asm_blink_cycle(void) {
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c0" : : "r" (LED_PIN), "r" (1));
    sleep_us(500 * 1000ull);
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c0" : : "r" (LED_PIN), "r" (0));
    sleep_us(500 * 1000ull);
}

int main(void) {
    asm_set_oe_and_out();
    asm_configure_pad();
    asm_set_funcsel();
    asm_clear_iso();
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c4" : : "r" (LED_PIN), "r" (GPIO_OUT));
    while (true) {
        asm_blink_cycle();
    }
}

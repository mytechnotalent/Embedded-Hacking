/**
 * @file 0x000b_integer-data-type.c
 * @brief Integer data types: cycle LEDs on GPIO16-18 with asm, print int values
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
 * Demonstrates uint8_t and int8_t integer data types. Initializes GPIO16-18
 * via inline assembly (PADS_BANK0, IO_BANK0, coprocessor OE), then cycles
 * LEDs using coprocessor mcrr instructions while printing integer values.
 *
 * Wiring:
 *   GPIO16 -> LED1 anode (with current-limiting resistor to GND)
 *   GPIO17 -> LED2 anode (with current-limiting resistor to GND)
 *   GPIO18 -> LED3 anode (with current-limiting resistor to GND)
 */

#include <stdio.h>
#include "pico/stdlib.h"

/**
 * @brief Initialize GPIO16-18 as SIO outputs via inline assembly loop
 *
 * @details Configures PADS_BANK0 (clear OD+ISO, set IE), IO_BANK0
 *          (FUNCSEL=5 SIO), and coprocessor OE for pins 16-18.
 *
 * @retval None
 */
static void asm_init_gpio_range(void) {
    __asm volatile (
        "ldr r3, =0x40038000\n"             // address of PADS_BANK0_BASE
        "ldr r2, =0x40028004\n"             // address of IO_BANK0 GPIO0.ctrl
        "movs r0, #16\n"                    // GPIO16 (start pin)
        "init_loop:\n"                      // loop start
            "lsls r1, r0, #2\n"             // pin * 4 (pad offset)
            "adds r4, r3, r1\n"             // PADS base + offset
            "ldr  r5, [r4]\n"               // load current config
            "bic  r5, r5, #0x180\n"         // clear OD+ISO
            "orr  r5, r5, #0x40\n"          // set IE
            "str  r5, [r4]\n"               // store updated config
            "lsls r1, r0, #3\n"             // pin * 8 (ctrl offset)
            "adds r4, r2, r1\n"             // IO_BANK0 base + offset
            "ldr  r5, [r4]\n"               // load current config
            "bic  r5, r5, #0x1f\n"          // clear FUNCSEL bits [4:0]
            "orr  r5, r5, #5\n"             // set FUNCSEL = 5 (SIO)
            "str  r5, [r4]\n"               // store updated config
            "mov  r4, r0\n"                 // pin
            "movs r5, #1\n"                 // bit 1; used for OUT/OE writes
            "mcrr p0, #4, r4, r5, c4\n"     // gpioc_bit_oe_put(pin,1)
            "adds r0, r0, #1\n"             // increment pin
            "cmp  r0, #20\n"                // stop after pin 18
            "blt  init_loop\n"              // loop until r0 == 20
    );
}

/**
 * @brief Blink a single pin using coprocessor mcrr instructions
 *
 * @details Turns the specified pin on for 500ms, then off for 500ms
 *          using inline asm mcrr coprocessor output instructions.
 *
 * @param pin GPIO pin number to blink
 * @retval None
 */
static void asm_blink_pin(uint8_t pin) {
    __asm volatile (
        "mov r4, %0\n"
        "movs r5, #0x01\n"
        "mcrr p0, #4, r4, r5, c0\n"
        : : "r"(pin) : "r4", "r5"
    );
    sleep_ms(500);
    __asm volatile (
        "mov r4, %0\n"
        "movs r5, #0\n"
        "mcrr p0, #4, r4, r5, c0\n"
        : : "r"(pin) : "r4", "r5"
    );
    sleep_ms(500);
}

/**
 * @brief Blink LED, advance pin, and print age/range
 *
 * @details Blinks the current pin, wraps pin 16-18,
 *          and prints both integer variables.
 *
 * @param pin   pointer to the current GPIO pin number
 * @param age   unsigned 8-bit age value
 * @param range signed 8-bit range value
 * @retval None
 */
static void blink_and_print(uint8_t *pin, uint8_t age, int8_t range) {
    asm_blink_pin(*pin);
    *pin = (*pin > 18) ? 16 : *pin + 1;
    printf("age: %d\r\n", age);
    printf("range: %d\r\n", range);
}

int main(void) {
    uint8_t age = 43;
    int8_t range = -42;
    stdio_init_all();
    asm_init_gpio_range();
    uint8_t pin = 16;
    while (1) {
        blink_and_print(&pin, age, range);
    }
}

/**
 * @file 0x0023_structures.c
 * @brief Structures: IR remote controls LEDs via a struct-based controller
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
 * Demonstrates C structures by defining a simple_led_ctrl_t to manage three
 * LEDs. An IR receiver on GPIO5 decodes NEC commands and activates the
 * corresponding LED (0x0C -> GPIO16, 0x18 -> GPIO17, 0x5E -> GPIO18).
 *
 * Wiring:
 *   GPIO5  -> IR receiver OUT
 *   GPIO16 -> LED1 anode (with current-limiting resistor to GND)
 *   GPIO17 -> LED2 anode (with current-limiting resistor to GND)
 *   GPIO18 -> LED3 anode (with current-limiting resistor to GND)
 */

#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "ir.h"

/** @brief GPIO pin number for the IR receiver */
#define IR_PIN 5

typedef struct {
    uint8_t led1_pin;
    uint8_t led2_pin;
    uint8_t led3_pin;
    bool led1_state;
    bool led2_state;
    bool led3_state;
} simple_led_ctrl_t;

/**
 * @brief Initialize all three LED GPIO pins as outputs
 *
 * @details Configures led1_pin, led2_pin, and led3_pin from the
 *          structure as GPIO outputs.
 *
 * @param leds pointer to the LED controller structure
 * @retval None
 */
static void init_led_gpios(simple_led_ctrl_t *leds) {
    gpio_init(leds->led1_pin);
    gpio_set_dir(leds->led1_pin, GPIO_OUT);
    gpio_init(leds->led2_pin);
    gpio_set_dir(leds->led2_pin, GPIO_OUT);
    gpio_init(leds->led3_pin);
    gpio_set_dir(leds->led3_pin, GPIO_OUT);
}

/**
 * @brief Process an NEC IR key and update LED states
 *
 * @details Maps NEC command codes to LEDs: 0x0C -> LED1, 0x18 -> LED2,
 *          0x5E -> LED3. Turns off all LEDs first, then activates the match.
 *
 * @param leds pointer to the LED controller structure
 * @param key  NEC command code from IR receiver
 * @retval None
 */
static void process_ir_key(simple_led_ctrl_t *leds, int key) {
    printf("NEC command: 0x%02X\n", key);
    leds->led1_state = (key == 0x0C);
    leds->led2_state = (key == 0x18);
    leds->led3_state = (key == 0x5E);
    gpio_put(leds->led1_pin, leds->led1_state);
    gpio_put(leds->led2_pin, leds->led2_state);
    gpio_put(leds->led3_pin, leds->led3_state);
    sleep_ms(10);
}

/**
 * @brief Poll IR receiver and dispatch key to handler
 *
 * @details Reads one IR key; if valid, processes it; otherwise
 *          yields with a short sleep.
 *
 * @param leds pointer to the LED controller structure
 * @retval None
 */
static void poll_ir(simple_led_ctrl_t *leds) {
    int key = ir_getkey();
    if (key >= 0) {
        process_ir_key(leds, key);
    } else {
        sleep_ms(1);
    }
}

/**
 * @brief Build the default LED controller structure
 *
 * @details Initializes pins 16-18 with all LEDs off.
 *
 * @return simple_led_ctrl_t initialized structure
 */
static simple_led_ctrl_t make_default_leds(void) {
    simple_led_ctrl_t leds = {
        .led1_pin = 16, 
        .led2_pin = 17, 
        .led3_pin = 18,
        .led1_state = false, 
        .led2_state = false, 
        .led3_state = false
    };
    return leds;
}

int main(void) {
    stdio_init_all();
    simple_led_ctrl_t leds = make_default_leds();
    init_led_gpios(&leds);
    ir_init(IR_PIN);
    printf("IR receiver on GPIO %d ready\n", IR_PIN);
    while (true) {
        poll_ir(&leds);
    }
}

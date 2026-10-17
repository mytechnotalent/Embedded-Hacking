/**
 * @file 0x0026_functions.c
 * @brief Functions: IR remote controls LEDs with helper function decomposition
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
 * Demonstrates function decomposition by breaking IR-controlled LED logic
 * into small, focused helper functions. An IR receiver on GPIO5 decodes
 * NEC commands and activates the corresponding LED with a blink effect.
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
 * @brief Map NEC IR command code to LED number
 *
 * @details Translates a received NEC IR command code to a logical
 *          LED number. Supports three button mappings.
 *
 * @param ir_command NEC command code from IR receiver
 * @retval 1-3 for matched LED, 0 if no match
 */
static int ir_to_led_number(int ir_command) {
    if (ir_command == 0x0C) return 1;
    if (ir_command == 0x18) return 2;
    if (ir_command == 0x5E) return 3;
    return 0;
}

/**
 * @brief Get GPIO pin number for a given LED number
 *
 * @details Retrieves the GPIO pin associated with a logical LED
 *          number from the LED controller structure.
 *
 * @param leds    pointer to LED controller structure
 * @param led_num LED number (1-3)
 * @retval GPIO pin number or 0 if invalid
 */
static uint8_t get_led_pin(simple_led_ctrl_t *leds, int led_num) {
    if (led_num == 1) return leds->led1_pin;
    if (led_num == 2) return leds->led2_pin;
    if (led_num == 3) return leds->led3_pin;
    return 0;
}

/**
 * @brief Turn off all LEDs in the controller
 *
 * @details Sets all three LED GPIO outputs to low.
 *
 * @param leds pointer to LED controller structure
 * @retval None
 */
static void leds_all_off(simple_led_ctrl_t *leds) {
    gpio_put(leds->led1_pin, false);
    gpio_put(leds->led2_pin, false);
    gpio_put(leds->led3_pin, false);
}

/**
 * @brief Blink an LED pin a specified number of times
 *
 * @details Toggles the specified GPIO pin on and off for the given
 *          count, with configurable delay between transitions.
 *
 * @param pin      GPIO pin number to blink
 * @param count    number of blink cycles
 * @param delay_ms delay in milliseconds for on/off periods
 * @retval None
 */
static void blink_led(uint8_t pin, uint8_t count, uint32_t delay_ms) {
    for (uint8_t i = 0; i < count; i++) {
        gpio_put(pin, true);
        sleep_ms(delay_ms);
        gpio_put(pin, false);
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Process IR command and activate corresponding LED
 *
 * @details Turns off all LEDs, maps the command to an LED number,
 *          blinks it, then holds it steady.
 *
 * @param ir_command  NEC command code from IR receiver
 * @param leds        pointer to LED controller structure
 * @param blink_count number of blinks before steady state
 * @retval LED number activated (1-3), 0 if none, -1 if invalid
 */
static int process_ir_led_command(int ir_command, simple_led_ctrl_t *leds, uint8_t blink_count) {
    if (!leds || ir_command < 0) return -1;
    leds_all_off(leds);
    int led_num = ir_to_led_number(ir_command);
    if (led_num == 0) return 0;
    uint8_t pin = get_led_pin(leds, led_num);
    blink_led(pin, blink_count, 50);
    gpio_put(pin, true);
    return led_num;
}

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
 * @brief Poll IR and handle a single received key
 *
 * @details Reads an IR key, processes the command with 3 blinks,
 *          and prints the result.
 *
 * @param leds pointer to the LED controller structure
 * @retval None
 */
/**
 * @brief Handle a valid IR key press
 *
 * @details Prints the NEC command, processes it with 3 blinks,
 *          and reports the activated LED.
 *
 * @param leds pointer to the LED controller structure
 * @param key  NEC command code
 * @retval None
 */
static void handle_ir_key(simple_led_ctrl_t *leds, int key) {
    printf("NEC command: 0x%02X\n", key);
    int activated_led = process_ir_led_command(key, leds, 3);
    if (activated_led > 0) {
        printf("LED %d activated on GPIO %d\n", activated_led,
               get_led_pin(leds, activated_led));
    }
    sleep_ms(10);
}

/**
 * @brief Poll IR and handle a single received key
 *
 * @details Reads an IR key; if valid dispatches to handler,
 *          otherwise yields with a short sleep.
 *
 * @param leds pointer to the LED controller structure
 * @retval None
 */
static void poll_and_handle_ir(simple_led_ctrl_t *leds) {
    int key = ir_getkey();
    if (key >= 0) {
        handle_ir_key(leds, key);
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
        .led1_pin = 16, .led2_pin = 17, .led3_pin = 18,
        .led1_state = false, .led2_state = false, .led3_state = false
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
        poll_and_handle_ir(&leds);
    }
}

/**
 * @file led.c
 * @brief LED controller driven by decoded NEC IR commands.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "led.h"

/**
 * @brief Map an NEC command code to a logical LED number.
 * @param ir_command Decoded NEC command byte.
 * @return int Logical LED number 1-3, or 0 when the command is unmapped.
 */
static int led_number_for_command(int ir_command) {
    if (ir_command == 0x0C) return 1;
    if (ir_command == 0x18) return 2;
    if (ir_command == 0x5E) return 3;
    return 0;
}

/**
 * @brief Return the GPIO pin backing a logical LED number.
 * @param leds    Pointer to the LED controller.
 * @param led_num Logical LED number (1-3).
 * @return uint8_t GPIO pin number for the requested LED.
 */
static uint8_t led_pin_for(simple_led_ctrl_t *leds, int led_num) {
    if (led_num == 1) return leds->led1_pin;
    if (led_num == 2) return leds->led2_pin;
    return leds->led3_pin;
}

/**
 * @brief Turn every LED output off.
 * @param leds Pointer to the LED controller.
 * @return void No return value.
 */
static void led_all_off(simple_led_ctrl_t *leds) {
    gpio_put(leds->led1_pin, false);
    gpio_put(leds->led2_pin, false);
    gpio_put(leds->led3_pin, false);
}

/**
 * @brief Toggle a GPIO on and off for a number of blink cycles.
 * @param pin      GPIO pin to blink.
 * @param count    Number of on/off cycles.
 * @param delay_ms Milliseconds to hold each phase.
 * @return void No return value.
 */
static void led_blink(uint8_t pin, uint8_t count, uint32_t delay_ms) {
    for (uint8_t i = 0; i < count; i++) {
        gpio_put(pin, true);
        sleep_ms(delay_ms);
        gpio_put(pin, false);
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Blink an LED, then leave it lit.
 * @param leds    Pointer to the LED controller.
 * @param led_num Logical LED number to activate.
 * @return void No return value.
 */
static void led_activate(simple_led_ctrl_t *leds, int led_num) {
    uint8_t pin = led_pin_for(leds, led_num);
    led_blink(pin, 3, 50);
    gpio_put(pin, true);
    printf("LED %d activated on GPIO %d\n", led_num, pin);
}

simple_led_ctrl_t led_default(void) {
    simple_led_ctrl_t leds = {
        .led1_pin = 16, .led2_pin = 17, .led3_pin = 18,
        .led1_state = false, .led2_state = false, .led3_state = false
    };
    return leds;
}

void led_init(simple_led_ctrl_t *leds) {
    gpio_init(leds->led1_pin);
    gpio_set_dir(leds->led1_pin, GPIO_OUT);
    gpio_init(leds->led2_pin);
    gpio_set_dir(leds->led2_pin, GPIO_OUT);
    gpio_init(leds->led3_pin);
    gpio_set_dir(leds->led3_pin, GPIO_OUT);
}

void led_handle_ir(simple_led_ctrl_t *leds, int key) {
    if (key < 0) return;
    printf("NEC command: 0x%02X\n", key);
    led_all_off(leds);
    int led_num = led_number_for_command(key);
    if (led_num == 0) return;
    led_activate(leds, led_num);
}
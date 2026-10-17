/**
 * @file gpio_ctrl.c
 * @brief GPIO hardware control implementation for Operation Iron Net
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "gpio_ctrl.h"
#include "pico/stdlib.h"

/**
 * @brief Initialize the red and green status LED pins as outputs.
 * @param void No parameters.
 * @return void No return value.
 */
static void init_led_pins(void) {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
}

/**
 * @brief Initialize the payload marker pin as a low output.
 * @param void No parameters.
 * @return void No return value.
 */
static void init_payload_pin(void) {
    gpio_init(PAYLOAD_PIN);
    gpio_set_dir(PAYLOAD_PIN, GPIO_OUT);
    gpio_put(PAYLOAD_PIN, 0);
}

/**
 * @brief Initialize the tamper loop pin as a pulled-down input.
 * @param void No parameters.
 * @return void No return value.
 */
static void init_tamper_pin(void) {
    gpio_init(TAMPER_PIN);
    gpio_set_dir(TAMPER_PIN, GPIO_IN);
    gpio_pull_down(TAMPER_PIN);
}

void init_gpio(void) {
    init_led_pins();
    init_payload_pin();
    init_tamper_pin();
    set_led_armed();
}

void set_led_armed(void) {
    gpio_put(LED_RED_PIN, 1);
    gpio_put(LED_GREEN_PIN, 0);
}

void set_led_authorized(void) {
    gpio_put(LED_RED_PIN, 0);
    gpio_put(LED_GREEN_PIN, 1);
}

void set_led_tamper_flash(bool state) {
    gpio_put(LED_RED_PIN, state ? 1 : 0);
    gpio_put(LED_GREEN_PIN, 0);
}

bool is_tamper_wire_intact(void) {
    return gpio_get(TAMPER_PIN);
}

void payload_deploy(void) {
    gpio_put(PAYLOAD_PIN, 1);
    sleep_ms(250);
    gpio_put(PAYLOAD_PIN, 0);
}

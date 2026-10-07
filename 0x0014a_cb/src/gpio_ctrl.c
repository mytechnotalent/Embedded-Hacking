/**
 * @file gpio_ctrl.c
 * @brief GPIO hardware control implementation for Operation Zero Hour ESA
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "gpio_ctrl.h"
#include "pico/stdlib.h"

static void init_led_pins(void) {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
}

static void init_tamper_pin(void) {
    gpio_init(TAMPER_WIRE_PIN);
    gpio_set_dir(TAMPER_WIRE_PIN, GPIO_IN);
    gpio_pull_down(TAMPER_WIRE_PIN);
}

void init_gpio(void) {
    init_led_pins();
    init_tamper_pin();
    set_led_armed();
}

void set_led_armed(void) {
    gpio_put(LED_RED_PIN, 1);
    gpio_put(LED_GREEN_PIN, 0);
}

void set_led_safe(void) {
    gpio_put(LED_RED_PIN, 0);
    gpio_put(LED_GREEN_PIN, 1);
}

void set_led_tamper_flash(bool state) {
    gpio_put(LED_RED_PIN, state ? 1 : 0);
    gpio_put(LED_GREEN_PIN, 0);
}

bool is_tamper_wire_intact(void) {
    return gpio_get(TAMPER_WIRE_PIN);
}

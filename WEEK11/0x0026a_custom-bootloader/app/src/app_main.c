/**
 * @file app_main.c
 * @brief Application entry point: IR receiver drives the LED controller.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 *
 * -----------------------------------------------------------------------------
 *
 * This file is deliberately thin: it only wires the modules together and runs
 * the poll loop. The NEC IR decoder lives in ir.c, the LED controller lives in
 * led.c, and the AES-128-CBC protected banner lives in secret.c.
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
#include "led.h"
#include "secret.h"

/** @brief GPIO pin number for the IR receiver. */
#define IR_PIN 5

/**
 * @brief Poll the IR receiver and dispatch decoded keys forever.
 * @param leds Pointer to the LED controller to drive.
 * @return void Does not return.
 */
static void ir_poll_loop(simple_led_ctrl_t *leds) {
    while (true) {
        int key = ir_getkey();
        if (key >= 0) led_handle_ir(leds, key);
        else sleep_ms(1);
    }
}

int main(void) {
    stdio_init_all();
    secret_print_banner();
    simple_led_ctrl_t leds = led_default();
    led_init(&leds);
    ir_init(IR_PIN);
    printf("IR receiver on GPIO %d ready\n", IR_PIN);
    ir_poll_loop(&leds);
}
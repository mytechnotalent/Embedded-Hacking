/**
 * @file main.c
 * @brief Operation Iron Net Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "gpio_ctrl.h"
#include "dht11.h"
#include "logic.h"

/**
 * @brief Initialize stdio, GPIO, the sensor, and the strike controller.
 * @param void No parameters.
 * @return void No return value.
 */
static void init_system(void) {
    stdio_init_all();
    init_gpio();
    dht11_init();
    init_strike_controller();
}

/**
 * @brief Run the main event loop, servicing one sensor tick per iteration.
 * @param void No parameters.
 * @return void No return value.
 */
static void run_event_loop(void) {
    while (true) {
        evaluate_environment();
        sleep_ms(1500);
    }
}

int main(void) {
    init_system();
    run_event_loop();
    return 0;
}

/**
 * @file main.c
 * @brief Operation Zero Hour - Electronic Safe-and-Arm (ESA) Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "pico/stdlib.h"
#include "gpio_ctrl.h"
#include "safe_arm.h"

static void init_system(void) {
    stdio_init_all();
    init_gpio();
    init_safe_arm_controller();
}

static void run_event_loop(void) {
    while (true) {
        process_safe_arm_tick();
        sleep_ms(100);
    }
}

int main(void) {
    init_system();
    run_event_loop();
    return 0;
}

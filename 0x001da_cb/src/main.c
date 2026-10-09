/**
 * @file main.c
 * @brief Operation Broken Wing Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pwm_ctrl.h"
#include "state_machine.h"

int main(void) {
    stdio_init_all();
    init_servo();
    
    sleep_ms(2000);
    printf("SYSTEM BOOT: HEAVY LIFT UAS ONLINE.\r\n");
    
    while (true) {
        process_payload_state();
        sleep_ms(2000);
    }
    return 0;
}

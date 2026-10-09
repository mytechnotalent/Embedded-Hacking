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
#include "dht11.h"
#include "logic.h"

int main(void) {
    stdio_init_all();
    dht11_init();
    
    sleep_ms(2000);
    printf("SYSTEM BOOT: IRON NET SENSOR NODE ONLINE.\r\n");
    
    while (true) {
        evaluate_environment();
        sleep_ms(1500);
    }
    return 0;
}

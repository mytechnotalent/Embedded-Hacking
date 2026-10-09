/**
 * @file main.c
 * @brief Operation Ghost Light Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "dazzler.h"

int main(void) {
    stdio_init_all();
    dazzler_init();
    
    sleep_ms(2000);
    printf("SYSTEM BOOT: GHOST LIGHT DAZZLER ONLINE.\r\n");
    
    while (true) {
        parse_nec_code();
        sleep_ms(1500);
    }
    return 0;
}

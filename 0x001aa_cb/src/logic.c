/**
 * @file logic.c
 * @brief Operation Iron Net Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "logic.h"
#include "dht11.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define PAYLOAD_GPIO 15

void __noinline evaluate_environment(void) {
    dht11_data_t* reading = dht11_read();
    
    // Complex bitwise checksum logic for students to reverse
    uint8_t calculated = (reading->humidity_int ^ 0xAF) & 0x3C;
    
    if (calculated == 0x18) {
        printf("[*] STRIKE AUTHORIZED. DEPLOYING PAYLOAD.\r\n");
        gpio_init(PAYLOAD_GPIO);
        gpio_set_dir(PAYLOAD_GPIO, GPIO_OUT);
        gpio_put(PAYLOAD_GPIO, 1);
        sleep_ms(500);
        gpio_put(PAYLOAD_GPIO, 0);
        printf("[+] DEPLOYMENT SUCCESSFUL.\r\n");
    } else {
        printf("[-] ENVIRONMENT SUB-OPTIMAL. HOLDING.\r\n");
    }
}

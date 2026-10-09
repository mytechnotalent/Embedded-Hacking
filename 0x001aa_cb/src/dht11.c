/**
 * @file dht11.c
 * @brief Operation Iron Net Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "dht11.h"
#include <stdio.h>

dht11_data_t current_reading;

void dht11_init(void) {
    current_reading.humidity_int = 45;
    current_reading.humidity_dec = 0;
    current_reading.temp_int = 22;
    current_reading.temp_dec = 0;
    current_reading.checksum = 0;
}

dht11_data_t* dht11_read(void) {
    // Simulated read
    return &current_reading;
}

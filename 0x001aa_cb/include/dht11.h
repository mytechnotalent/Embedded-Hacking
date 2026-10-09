/**
 * @file dht11.h
 * @brief Operation Iron Net Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef DHT11_H
#define DHT11_H
#include <stdint.h>
typedef struct {
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t checksum;
} dht11_data_t;
void dht11_init(void);
dht11_data_t* dht11_read(void);
#endif

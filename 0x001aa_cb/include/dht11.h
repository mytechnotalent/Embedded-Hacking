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
#include <stdbool.h>

#define DHT11_PIN 14

typedef struct {
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t checksum;
    bool valid;
} dht11_data_t;

/**
 * @brief Initialize the DHT11 data pin and idle the sensor high.
 * @param void No parameters.
 * @return void No return value.
 */
void dht11_init(void);

/**
 * @brief Perform a single-wire DHT11 read into the reading cache.
 * @param void No parameters.
 * @return dht11_data_t* Pointer to the current sensor reading.
 */
dht11_data_t* dht11_read(void);

#endif // DHT11_H

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
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"

static dht11_data_t g_current_reading;

/**
 * @brief Block until the data pin reaches a level or a timeout expires.
 * @param level Logic level to wait for (true high, false low).
 * @param timeout_us Maximum time to wait in microseconds.
 * @return true when the level was observed, false on timeout.
 */
static bool dht11_wait_for(bool level, uint32_t timeout_us) {
    uint64_t start = time_us_64();
    while (gpio_get(DHT11_PIN) != level) {
        if ((uint32_t)(time_us_64() - start) > timeout_us) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Drive the 18 ms start pulse and wait for the sensor response.
 * @param void No parameters.
 * @return true when the response handshake was seen, false otherwise.
 */
static bool dht11_start_signal(void) {
    gpio_set_dir(DHT11_PIN, GPIO_OUT);
    gpio_put(DHT11_PIN, 0);
    sleep_ms(20);
    gpio_set_dir(DHT11_PIN, GPIO_IN);
    return dht11_wait_for(false, 100)
        && dht11_wait_for(true, 100)
        && dht11_wait_for(false, 100);
}

/**
 * @brief Measure a single high pulse and return its decoded bit value.
 * @param void No parameters.
 * @return 1 for a long pulse, 0 for a short pulse, or -1 on timeout.
 */
static int dht11_read_bit(void) {
    if (!dht11_wait_for(true, 100)) {
        return -1;
    }
    uint64_t high_start = time_us_64();
    if (!dht11_wait_for(false, 100)) {
        return -1;
    }
    return ((uint32_t)(time_us_64() - high_start) > 40) ? 1 : 0;
}

/**
 * @brief Sample the 40 data bits by measuring each high pulse width.
 * @param bytes Output buffer receiving the five decoded bytes.
 * @return true when all forty bits were captured, false on timeout.
 */
static bool dht11_capture_bits(uint8_t *bytes) {
    for (int i = 0; i < 40; i++) {
        int bit = dht11_read_bit();
        if (bit < 0) {
            return false;
        }
        bytes[i / 8] = (uint8_t)((bytes[i / 8] << 1) | bit);
    }
    return true;
}

/**
 * @brief Verify the frame checksum equals the low byte of the data sum.
 * @param bytes Pointer to the five decoded frame bytes.
 * @return true when the checksum matches, false otherwise.
 */
static bool dht11_checksum_ok(const uint8_t *bytes) {
    uint8_t sum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
    return sum == bytes[4];
}

/**
 * @brief Copy the validated frame bytes into the reading structure.
 * @param out Pointer to the reading structure to populate.
 * @param bytes Pointer to the five decoded frame bytes.
 * @return void No return value.
 */
static void dht11_store_bytes(dht11_data_t *out, const uint8_t *bytes) {
    out->humidity_int = bytes[0];
    out->humidity_dec = bytes[1];
    out->temp_int = bytes[2];
    out->temp_dec = bytes[3];
    out->checksum = bytes[4];
    out->valid = true;
}

/**
 * @brief Run the start handshake and capture the five frame bytes.
 * @param bytes Output buffer receiving the five decoded bytes.
 * @return true when the handshake and capture both succeeded.
 */
static bool dht11_collect_frame(uint8_t *bytes) {
    if (!dht11_start_signal()) {
        return false;
    }
    return dht11_capture_bits(bytes);
}

/**
 * @brief Issue the start pulse and reconstruct the five sensor bytes.
 * @param out Pointer to the reading structure to populate.
 * @return true when a valid checksum frame was decoded, false otherwise.
 */
static bool dht11_read_frame(dht11_data_t *out) {
    uint8_t bytes[5] = {0};
    if (!dht11_collect_frame(bytes)) {
        return false;
    }
    if (dht11_checksum_ok(bytes)) {
        dht11_store_bytes(out, bytes);
        return true;
    }
    return false;
}

void dht11_init(void) {
    gpio_init(DHT11_PIN);
    gpio_set_dir(DHT11_PIN, GPIO_OUT);
    gpio_put(DHT11_PIN, 1);
    g_current_reading.valid = false;
}

dht11_data_t* dht11_read(void) {
    g_current_reading.valid = false;
    if (!dht11_read_frame(&g_current_reading)) {
        g_current_reading.valid = false;
    }
    return &g_current_reading;
}

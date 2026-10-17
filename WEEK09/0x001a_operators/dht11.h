/**
 * @file dht11.h
 * @brief Header for DHT11 temperature and humidity sensor driver
 * @author Kevin Thomas
 * @date 2025
 * 
 * MIT License
 * 
 * Copyright (c) 2025 Kevin Thomas
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the DHT11 driver
 *
 * Configures the GPIO pin for the DHT11 sensor. This must be called before
 * using dht11_read().
 *
 * @param pin GPIO pin number connected to DHT11 signal
 */
void dht11_init(uint8_t pin);

/**
 * @brief Read temperature and humidity from DHT11 sensor
 *
 * Performs the DHT11 communication protocol to read sensor data.
 *
 * @param humidity Pointer to store humidity value (0-100%)
 * @param temperature Pointer to store temperature value in Celsius
 * @return true if read successful, false on error or timeout
 */
bool dht11_read(float *humidity, float *temperature);

#endif // DHT11_H

/**
 * @file adc.h
 * @brief Header for 12-bit ADC driver (analog pin + internal temperature sensor)
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

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/**
 * @brief Initialize the ADC peripheral and configure an analog GPIO pin
 *
 * Powers on the ADC block, configures the specified GPIO as an analog input,
 * enables the internal temperature sensor on channel 4, and selects the
 * given channel as the active input. Must be called once before using any
 * other adc_driver_* functions.
 *
 * @param gpio    GPIO pin number for the analog input (26 = ch0, 27 = ch1, 28 = ch2)
 * @param channel ADC channel number corresponding to the GPIO (0, 1, or 2)
 */
void adc_driver_init(uint32_t gpio, uint8_t channel);

/**
 * @brief Perform a single ADC conversion on the active channel and return millivolts
 *
 * Reads the 12-bit raw value from the currently selected channel and scales
 * it to millivolts using the 3.3 V (3300 mV) full-scale reference.
 *
 * @return uint32_t Measured voltage in millivolts (0 to 3300)
 */
uint32_t adc_driver_read_mv(void);

/**
 * @brief Read the on-chip temperature sensor and return degrees Celsius
 *
 * Temporarily switches to ADC channel 4 (internal temperature sensor),
 * performs a conversion, applies the formula from the RP2350 datasheet,
 * then restores the channel that was active before the call.
 *
 * @return float Die temperature in degrees Celsius
 */
float adc_driver_read_temp_celsius(void);

#endif // ADC_H

/**
 * @file adc.c
 * @brief Implementation of the 12-bit ADC driver
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

#include "adc.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_VREF_MV    3300
#define ADC_FULL_SCALE 4095

static uint8_t active_channel = 0;

/**
 * @brief Convert a raw 12-bit ADC value to millivolts
 *
 * Scales the raw value linearly against the 3.3 V reference.
 *
 * @param raw 12-bit ADC conversion result (0 - 4095)
 * @return uint32_t Equivalent voltage in millivolts (0 - 3300)
 */
static uint32_t raw_to_mv(uint16_t raw) {
    return (uint32_t)raw * ADC_VREF_MV / ADC_FULL_SCALE;
}

/**
 * @brief Convert a raw temperature-sensor ADC value to degrees Celsius
 *
 * Applies the RP2350 datasheet formula:
 *   T = 27 - (V - 0.706) / 0.001721
 *
 * @param raw 12-bit ADC result from the internal temperature sensor (channel 4)
 * @return float Die temperature in degrees Celsius
 */
static float raw_to_celsius(uint16_t raw) {
    float voltage = (float)raw * 3.3f / (float)ADC_FULL_SCALE;
    return 27.0f - (voltage - 0.706f) / 0.001721f;
}

void adc_driver_init(uint32_t gpio, uint8_t channel) {
    active_channel = channel;
    adc_init();
    adc_gpio_init(gpio);
    adc_set_temp_sensor_enabled(true);
    adc_select_input(channel);
}

uint32_t adc_driver_read_mv(void) {
    return raw_to_mv(adc_read());
}

float adc_driver_read_temp_celsius(void) {
    adc_select_input(4);
    float result = raw_to_celsius(adc_read());
    adc_select_input(active_channel);
    return result;
}

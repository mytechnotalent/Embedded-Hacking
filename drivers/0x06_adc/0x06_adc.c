/**
 * @file 0x06_adc.c
 * @brief ADC demonstration: potentiometer voltage + on-chip temperature
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
 *
 * -----------------------------------------------------------------------------
 *
 * Demonstrates 12-bit ADC using the adc driver (adc.h / adc.c). Reads
 * ADC channel 0 (GPIO 26) and reports the voltage in millivolts alongside
 * the on-chip temperature sensor reading every 500 ms over UART.
 *
 * Wiring:
 *   GPIO26 -> Wiper of a 10 kohm potentiometer
 *   3.3V   -> One end of the potentiometer
 *   GND    -> Other end of the potentiometer
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "adc.h"

#define ADC_GPIO    26
#define ADC_CHANNEL 0

int main(void) {
    stdio_init_all();
    adc_driver_init(ADC_GPIO, ADC_CHANNEL);

    printf("ADC driver initialized: GPIO%d (channel %d)\r\n",
           ADC_GPIO, ADC_CHANNEL);

    while (true) {
        uint32_t voltage_mv = adc_driver_read_mv();
        float    temp_c     = adc_driver_read_temp_celsius();

        printf("ADC0: %4lu mV  |  Chip temp: %.1f C\r\n",
               voltage_mv, temp_c);

        sleep_ms(500);
    }
}

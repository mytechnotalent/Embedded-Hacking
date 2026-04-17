/**
 * @file rp2350_dht11.h
 * @brief DHT11 temperature and humidity sensor driver for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Single-wire protocol driver for the DHT11 sensor on GPIO4.
 * Uses SIO for GPIO direction switching and TIMER0 TIMERAWL
 * for microsecond pulse-width measurement.
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
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
#ifndef __RP2350_DHT11_H
#define __RP2350_DHT11_H

#include "rp2350.h"

/**
  * @brief  Release TIMER0 from reset in the reset controller.
  * @retval None
  */
void dht11_timer_release_reset(void);

/**
  * @brief  Start the TIMER0 tick generator for 1 us ticks at 12 MHz.
  * @retval None
  */
void dht11_timer_start_tick(void);

/**
  * @brief  Configure GPIO4 pad and funcsel for SIO with pull-up.
  * @retval None
  */
void dht11_init(void);

/**
  * @brief  Read temperature and humidity from the DHT11 sensor.
  * @param  humidity    pointer to store humidity integer percentage
  * @param  temperature pointer to store temperature integer Celsius
  * @retval bool true on success, false on timeout or checksum error
  */
bool dht11_read(uint8_t *humidity, uint8_t *temperature);

#endif /* __RP2350_DHT11_H */

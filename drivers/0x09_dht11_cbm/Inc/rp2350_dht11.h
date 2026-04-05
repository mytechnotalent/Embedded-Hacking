/**
  ******************************************************************************
  * @file    rp2350_dht11.h
  * @author  Kevin Thomas
  * @brief   DHT11 temperature and humidity sensor driver for RP2350.
  *
  *          Single-wire protocol driver for the DHT11 sensor on GPIO4.
  *          Uses SIO for GPIO direction switching and TIMER0 TIMERAWL
  *          for microsecond pulse-width measurement.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
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

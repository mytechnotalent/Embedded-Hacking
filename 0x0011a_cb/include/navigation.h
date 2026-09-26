// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    navigation.h
// Desc:    Declares autonomous guidance, dead-reckoning, and telemetry interface.
// Created: 2026

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <stdbool.h>

/** @brief Origin recovery coordinates in Centreville, VA */
#define START_LAT 38.840280
#define START_LON -77.428890

/** @brief Target coordinates at NRO Headquarters (Chantilly, VA) */
extern const double TARGET_LAT;
extern const double TARGET_LON;

/**
 * @brief Transmit telemetry stream over Debug UART0 and LoRa UART1.
 *
 * @param cur_lat Current micro-UAV latitude.
 * @param cur_lon Current micro-UAV longitude.
 * @return None.
 */
void send_telemetry(double cur_lat, double cur_lon);

/**
 * @brief Advance dead-reckoning position toward programmed waypoint.
 *
 * @param cur_lat Pointer to current latitude.
 * @param cur_lon Pointer to current longitude.
 * @return None.
 */
void dead_reckon_step(double *cur_lat, double *cur_lon);

/**
 * @brief Verify if micro-UAV has arrived at target coordinates.
 *
 * @param cur_lat Current latitude coordinate.
 * @param cur_lon Current longitude coordinate.
 * @return true if arrived at target, false otherwise.
 */
bool check_arrival(double cur_lat, double cur_lon);

/**
 * @brief Manage guidance progression, propeller oscillation, and payload release.
 *
 * @param cur_lat Current latitude coordinate.
 * @param cur_lon Current longitude coordinate.
 * @return None.
 */
void navigate_to_target(double cur_lat, double cur_lon);

#endif // NAVIGATION_H

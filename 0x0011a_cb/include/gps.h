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
// File:    gps.h
// Desc:    Declares PIO UART GPS receiver interface and NMEA parsing logic.
// Created: 2026

#ifndef GPS_H
#define GPS_H

#include "hardware/pio.h"

#define GPS_PIN       7
#define GPS_BAUD      9600
#define GPS_PIO       pio0
#define GPS_SM        0

/**
 * @brief Initialize PIO UART receiver on GPIO7 for u-blox NEO-6M GPS.
 *
 * @param None.
 * @return None.
 */
void init_gps_pio(void);

/**
 * @brief Poll PIO RX FIFO and parse incoming NMEA GPS coordinates.
 *
 * @param lat Pointer to double storing updated latitude.
 * @param lon Pointer to double storing updated longitude.
 * @return None.
 */
void poll_gps(double *lat, double *lon);

#endif // GPS_H

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
// File:    payload.h
// Desc:    Declares payload release mechanism interface on GPIO16.
// Created: 2026

#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdbool.h>

#define LED_RED_PIN    16
#define LED_GREEN_PIN  17
#define LED_YELLOW_PIN 18

/**
 * @brief Initialize GPIO16 (Red failure LED) and GPIO17 (Green success LED).
 *
 * @param None.
 * @return None.
 */
void init_payload(void);

/**
 * @brief Update tri-color GNSS status LEDs from fix state and satellites.
 *
 * Red (GP16) = no satellites in view; Yellow (GP18) = satellites in view
 * while acquiring; Green (GP17) = active 3D fix.
 *
 * @param fix True if an active 3D GPS fix is held.
 * @param siv Number of satellites currently in view.
 * @return None.
 */
void set_gnss_leds(bool fix, int siv);

/**
 * @brief Energize payload latch and illuminate both LEDs at target coordinates.
 *
 * @param None.
 * @return None.
 */
void release_payload(void);

#endif // PAYLOAD_H


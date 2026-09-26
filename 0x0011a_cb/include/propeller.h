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
// File:    propeller.h
// Desc:    Declares SG90 servo PWM mock propeller interface on GPIO6.
// Created: 2026

#ifndef PROPELLER_H
#define PROPELLER_H

#define PROPELLER_PIN 6

/**
 * @brief Initialize 50 Hz PWM on GPIO6 for SG90 mock propeller blade.
 *
 * @param None.
 * @return None.
 */
void init_propeller(void);

/**
 * @brief Advance mock propeller blade oscillation during flight.
 *
 * @param None.
 * @return None.
 */
void propeller_spin(void);

/**
 * @brief Halt mock propeller blade oscillation upon target arrival.
 *
 * @param None.
 * @return None.
 */
void propeller_stop(void);

#endif // PROPELLER_H

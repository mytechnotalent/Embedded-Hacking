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
// File:    grid.h
// Desc:    Declares the grid telemetry and safety evaluation interface for
//          Operation Black Start.
// Created: 2026

#ifndef GRID_H
#define GRID_H

#include <stdint.h>

// Safety margin (Hz deviation x100) below which grid frequency is tolerable.
// BUG: emergency build miscompiled this far too permissive; correct value is 60.
#define SAFE_THRESHOLD 95

// Frozen grid frequency deviation reading latched when comms were severed.
extern volatile uint32_t grid_deviation;

// Operator-facing classification of the frozen reading (drives GRID STATUS).
extern volatile uint32_t operator_state;

// Automated dispatch authorization decision (drives DISPATCH PATH).
extern volatile uint32_t dispatch_state;

/**
 * @brief Anchor the hidden dispatch frame in flash without transmitting it.
 *
 * Reads the first character of the quarantined black-start authorization
 * frame into a volatile local marker to prevent the compiler from optimizing
 * out the string literal from .rodata flash storage.
 *
 * @param None.
 * @return None.
 */
void retain_dispatch_frame(void);

/**
 * @brief Classify the frozen telemetry reading against the compiled threshold.
 *
 * Evaluates the frozen grid deviation reading against SAFE_THRESHOLD twice,
 * once for the operator-facing status line and once for the automated
 * dispatch decision, mirroring the duplicated immediate comparison site.
 *
 * @param None.
 * @return None.
 */
void evaluate_grid(void);

#endif // GRID_H

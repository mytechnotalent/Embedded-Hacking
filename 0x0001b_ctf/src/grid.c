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
// File:    grid.c
// Desc:    Implements the grid telemetry and safety evaluation logic for
//          Operation Black Start.
// Created: 2026

#include "grid.h"
#include <stdint.h>

// Frozen grid frequency deviation reading latched when comms were severed.
volatile uint32_t grid_deviation = 87;

// Operator-facing classification of the frozen reading (drives GRID STATUS).
volatile uint32_t operator_state = 0;

// Automated dispatch authorization decision (drives DISPATCH PATH).
volatile uint32_t dispatch_state = 0;

// Quarantined black-start authorization frame, retained in flash, never sent.
static volatile const char dispatch_frame[] =
    "WORLDGRID:BLACKSTART:GRID-7:WATER-3";

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
void retain_dispatch_frame(void)
{
    volatile char frame_marker = dispatch_frame[0];
    (void)frame_marker;
}

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
void evaluate_grid(void)
{
    operator_state = (grid_deviation < SAFE_THRESHOLD) ? 1 : 0;
    dispatch_state = (grid_deviation < SAFE_THRESHOLD) ? 1 : 0;
}

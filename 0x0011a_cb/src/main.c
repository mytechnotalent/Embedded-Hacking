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
// File:    main.c
// Desc:    Main entry point for autonomous micro-UAV guidance firmware.
// Created: 2026

#include "gps.h"
#include "lora.h"
#include "payload.h"
#include "propeller.h"
#include "navigation.h"
#include "pico/stdlib.h"

/**
 * @brief Initialize all board peripherals, communications, and actuators.
 *
 * @param None.
 * @return None.
 */
static void init_all(void)
{
    stdio_init_all();
    init_lora();
    init_gps_pio();
    init_payload();
    init_propeller();
}

/**
 * @brief Autonomous micro-UAV firmware execution loop.
 *
 * @param None.
 * @return int Standard exit code (never reached in embedded firmware).
 */
int main(void)
{
    double cur_lat = START_LAT;
    double cur_lon = START_LON;
    init_all();
    while (true) {
        poll_gps(&cur_lat, &cur_lon);
        dead_reckon_step(&cur_lat, &cur_lon);
        navigate_to_target(cur_lat, cur_lon);
        sleep_ms(1000);
    }
    return 0;
}

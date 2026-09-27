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
#include "lcd.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include <stdio.h>

/**
 * @brief Initialize all board peripherals, communications, and actuators.
 *
 * @param None.
 * @return None.
 */
static void init_all(void)
{
    stdio_init_all();
    init_navigation();
    init_payload();
    init_lora();
    init_gps_pio();
    init_propeller();
    init_lcd();
}

/**
 * @brief Continually drain GPS PIO FIFO over 1-second flight tick.
 *
 * @param cur_lat Pointer to current latitude.
 * @param cur_lon Pointer to current longitude.
 * @return bool True if active 3D lock was parsed, false otherwise.
 */
static bool update_position(double *cur_lat, double *cur_lon)
{
    bool got_fix = false;
    for (int i = 0; i < 200; i++, sleep_ms(5)) {
        got_fix |= poll_gps(cur_lat, cur_lon);
        lora_tick();
    }
    return got_fix;
}

static void step_mission(double *cur_lat, double *cur_lon)
{
    int siv = 0, cno = 0;
    static int hold = 0;
    bool fix = update_position(cur_lat, cur_lon);
    gps_get_stats(&siv, &cno);
    hold = fix ? 3 : ((hold > 0) ? (hold - 1) : 0);
    bool have = fix || (hold > 0);
    set_gnss_leds(have, siv);
    if (have) {
        lcd_show_coords(*cur_lat, *cur_lon);
        navigate_to_target(*cur_lat, *cur_lon);
    } else {
        lcd_show_gnss(siv, cno);
        propeller_stop();
        send_telemetry(*cur_lat, *cur_lon);
    }
}

/**
 * @brief Autonomous micro-UAV firmware execution loop.
 *
 * @param None.
 * @return int Standard exit code (never reached in embedded firmware).
 */
int main(void)
{
    double cur_lat = ORIGIN_LAT, cur_lon = ORIGIN_LON;
    init_all();
    while (true)
        step_mission(&cur_lat, &cur_lon);
    return 0;
}


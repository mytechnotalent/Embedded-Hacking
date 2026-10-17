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

/**
 * @brief Update the fix-hold countdown from the latest poll result.
 *
 * @param fix True when an active 3D lock was parsed this tick.
 * @return bool True when a fix is current or still held.
 */
static bool track_fix(bool fix)
{
    static int hold = 0;
    hold = fix ? 3 : ((hold > 0) ? (hold - 1) : 0);
    return fix || (hold > 0);
}

/**
 * @brief Execute guidance actions while a position fix is held.
 *
 * @param cur_lat Pointer to current latitude.
 * @param cur_lon Pointer to current longitude.
 * @return None.
 */
static void run_guided(double *cur_lat, double *cur_lon)
{
    lcd_show_coords(*cur_lat, *cur_lon);
    navigate_to_target(*cur_lat, *cur_lon);
}

/**
 * @brief Execute fallback actions while no position fix is held.
 *
 * @param siv Satellites in view.
 * @param cno Best carrier-to-noise density.
 * @param cur_lat Pointer to current latitude.
 * @param cur_lon Pointer to current longitude.
 * @return None.
 */
static void run_unguided(int siv, int cno, double *cur_lat, double *cur_lon)
{
    lcd_show_gnss(siv, cno);
    propeller_stop();
    send_telemetry(*cur_lat, *cur_lon);
}

/**
 * @brief Advance the mission by one flight tick.
 *
 * @param cur_lat Pointer to current latitude.
 * @param cur_lon Pointer to current longitude.
 * @return None.
 */
static void step_mission(double *cur_lat, double *cur_lon)
{
    bool have = track_fix(update_position(cur_lat, cur_lon));
    int siv = 0, cno = 0;
    gps_get_stats(&siv, &cno);
    set_gnss_leds(have, siv);
    if (have)
        run_guided(cur_lat, cur_lon);
    else
        run_unguided(siv, cno, cur_lat, cur_lon);
}

int main(void)
{
    double cur_lat = ORIGIN_LAT, cur_lon = ORIGIN_LON;
    init_all();
    while (true)
        step_mission(&cur_lat, &cur_lon);
    return 0;
}


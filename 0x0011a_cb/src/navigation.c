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
// File:    navigation.c
// Desc:    Implements waypoint navigation, dead-reckoning, and telemetry dispatch.
// Created: 2026

#include "navigation.h"
#include "lora.h"
#include "payload.h"
#include "propeller.h"
#include <stdio.h>
#include <math.h>

const double TARGET_LAT = 38.881940;
const double TARGET_LON = -77.450280;

void send_telemetry(double cur_lat, double cur_lon)
{
    char msg[80];
    snprintf(msg, sizeof(msg), "CURRENT LAT: %lf, LON: %lf\r\n", cur_lat, cur_lon);
    printf("%s", msg);
    lora_send(msg);
    snprintf(msg, sizeof(msg), "TARGET LAT:  %lf, LON: %lf\r\n", TARGET_LAT, TARGET_LON);
    printf("%s", msg);
    lora_send(msg);
}

void dead_reckon_step(double *cur_lat, double *cur_lon)
{
    double dlat = TARGET_LAT - *cur_lat;
    double dlon = TARGET_LON - *cur_lon;
    *cur_lat += (fabs(dlat) < 0.005) ? dlat : ((dlat > 0.0) ? 0.004166 : -0.004166);
    *cur_lon += (fabs(dlon) < 0.005) ? dlon : ((dlon > 0.0) ? 0.002139 : -0.002139);
}

bool check_arrival(double cur_lat, double cur_lon)
{
    return (cur_lat == TARGET_LAT) && (cur_lon == TARGET_LON);
}

void navigate_to_target(double cur_lat, double cur_lon)
{
    send_telemetry(cur_lat, cur_lon);
    if (check_arrival(cur_lat, cur_lon)) {
        propeller_stop();
        release_payload();
    } else {
        propeller_spin();
    }
}

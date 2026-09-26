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
// File:    gps.c
// Desc:    Implements PIO UART GPS receiver and NMEA coordinate parsing.
// Created: 2026

#include "gps.h"
#include "uart_rx.pio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_gps_pio(void)
{
    uint offset = pio_add_program(GPS_PIO, &uart_rx_program);
    uart_rx_program_init(GPS_PIO, GPS_SM, offset, GPS_PIN, GPS_BAUD);
}

/**
 * @brief Convert NMEA ddmm.mmmm coordinate to decimal degrees.
 *
 * @param str NMEA coordinate string.
 * @param dir Cardinal direction character ('N', 'S', 'E', 'W').
 * @return double Decimal degree coordinate.
 */
static double parse_nmea_coord(const char *str, char dir)
{
    double raw = atof(str);
    int deg = (int)(raw / 100.0);
    double dec = (double)deg + ((raw - (deg * 100.0)) / 60.0);
    return ((dir == 'S') || (dir == 'W')) ? -dec : dec;
}

/**
 * @brief Find pointer to n-th comma-separated field in NMEA string.
 *
 * @param str NMEA sentence string.
 * @param field_idx Index of field to locate.
 * @return const char* Pointer to field start or NULL.
 */
static const char *get_nmea_field(const char *str, int field_idx)
{
    while ((str != NULL) && (*str != '\0') && (field_idx > 0)) {
        if (*str++ == ',') {
            field_idx--;
        }
    }
    return (field_idx == 0) ? str : NULL;
}

/**
 * @brief Parse NMEA RMC sentence for valid coordinates.
 *
 * @param line NMEA sentence buffer.
 * @param lat Pointer to store parsed latitude.
 * @param lon Pointer to store parsed longitude.
 * @return None.
 */
static void parse_rmc(const char *line, double *lat, double *lon)
{
    const char *st = get_nmea_field(line, 2);
    const char *la = get_nmea_field(line, 3);
    const char *ns = get_nmea_field(line, 4);
    const char *lo = get_nmea_field(line, 5);
    const char *ew = get_nmea_field(line, 6);
    if ((st != NULL) && (*st == 'A') && (la != NULL) && (lo != NULL)) {
        *lat = parse_nmea_coord(la, *ns);
        *lon = parse_nmea_coord(lo, *ew);
    }
}

/**
 * @brief Accumulate GPS character and trigger RMC parsing on newline.
 *
 * @param ch Received ASCII character.
 * @param lat Pointer to current latitude.
 * @param lon Pointer to current longitude.
 * @return None.
 */
static void process_gps_char(char ch, double *lat, double *lon)
{
    static char buf[96];
    static int idx = 0;
    if ((ch == '\n') || (ch == '\r')) {
        buf[idx] = '\0';
        idx = 0;
        if (strstr(buf, "RMC") != NULL) {
            parse_rmc(buf, lat, lon);
        }
    } else if (idx < (int)(sizeof(buf) - 1)) {
        buf[idx++] = ch;
    }
}

void poll_gps(double *lat, double *lon)
{
    while (!pio_sm_is_rx_fifo_empty(GPS_PIO, GPS_SM)) {
        char ch = (char)(pio_sm_get(GPS_PIO, GPS_SM) >> 24);
        process_gps_char(ch, lat, lon);
    }
}

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

static int gps_siv = 0;
static int gps_cno = 0;

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
static bool parse_rmc(const char *line, double *lat, double *lon)
{
    const char *st = get_nmea_field(line, 2), *la = get_nmea_field(line, 3);
    const char *lo = get_nmea_field(line, 5);
    if (!st || *st != 'A' || !la || *la == ',' || !lo || *lo == ',') return false;
    *lat = parse_nmea_coord(la, *get_nmea_field(line, 4));
    *lon = parse_nmea_coord(lo, *get_nmea_field(line, 6));
    return (*lat != 0.0) && (*lon != 0.0);
}

/**
 * @brief Parse NMEA GSV sentence for satellites-in-view and best C/N0.
 *
 * @param line NMEA GSV sentence string.
 * @return None.
 */
static void parse_gsv(const char *line)
{
    const char *sv = get_nmea_field(line, 3), *msg = get_nmea_field(line, 2);
    if (sv != NULL) gps_siv = atoi(sv);
    if ((msg != NULL) && (*msg == '1')) gps_cno = 0;
    for (int f = 7; f < 40; f += 4) {
        const char *c = get_nmea_field(line, f);
        if ((c == NULL) || (*c == '*') || (*c == '\0')) break;
        if (atoi(c) > gps_cno) gps_cno = atoi(c);
    }
}

/**
 * @brief Test buffered line for valid NMEA RMC sentence.
 *
 * @param buf NMEA character buffer.
 * @param idx Pointer to character index.
 * @param lat Pointer to current latitude.
 * @param lon Pointer to current longitude.
 * @return bool True if valid 3D fix was parsed, false otherwise.
 */
static bool check_rmc_line(char *buf, int *idx, double *lat, double *lon)
{
    buf[*idx] = '\0';
    *idx = 0;
    char *rmc = strstr(buf, "RMC"), *gsv = strstr(buf, "GSV");
    if (gsv != NULL) parse_gsv(gsv);
    return (rmc != NULL) ? parse_rmc(rmc, lat, lon) : false;
}

/**
 * @brief Accumulate GPS character and trigger RMC parsing on newline.
 *
 * @param ch Received ASCII character.
 * @param lat Pointer to current latitude.
 * @param lon Pointer to current longitude.
 * @return bool True if a valid active 3D fix was parsed, false otherwise.
 */
static bool process_gps_char(char ch, double *lat, double *lon)
{
    static char buf[96];
    static int idx = 0;
    if (ch == '$') idx = 0;
    if ((ch == '\n') || (ch == '\r'))
        return check_rmc_line(buf, &idx, lat, lon);
    if (idx < (int)(sizeof(buf) - 1))
        buf[idx++] = ch;
    return false;
}

static bool handle_gps_byte(double *lat, double *lon)
{
    char ch = (char)(pio_sm_get(GPS_PIO, GPS_SM) >> 24);
    return process_gps_char(ch, lat, lon);
}

bool poll_gps(double *lat, double *lon)
{
    bool got_fix = false;
    while (!pio_sm_is_rx_fifo_empty(GPS_PIO, GPS_SM))
        got_fix |= handle_gps_byte(lat, lon);
    return got_fix;
}

void gps_get_stats(int *siv, int *cno)
{
    *siv = gps_siv;
    *cno = gps_cno;
}


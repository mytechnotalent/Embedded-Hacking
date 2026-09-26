#!/usr/bin/env python3
# MIT License
#
# Copyright (c) 2026 Kevin Thomas
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Author:  Kevin Thomas
# Email:   kevin@mytechnotalent.com
# GitHub:  https://github.com/mytechnotalent
# File:    telemetry_monitor.py
# Desc:    Real-time telemetry monitor for Operation Dark Vector.
# Created: 2026

"""
Real-time telemetry monitor for Operation Dark Vector.

Reads live avionics and GPS telemetry from the FT232RL USB-to-UART ground
station bridge, parses coordinates and distance, and prints a mission status
dashboard to the terminal.
"""

import argparse
import math
import sys
import time

try:
    import serial
except ImportError:
    serial = None


def _calculate_distance(
    lat1: float,
    lon1: float,
    lat2: float,
    lon2: float
) -> float:
    """
    Calculate approximate Euclidean distance in kilometers.

    Parameters
    ----------
    lat1 : float
        Current latitude in degrees.
    lon1 : float
        Current longitude in degrees.
    lat2 : float
        Target latitude in degrees.
    lon2 : float
        Target longitude in degrees.

    Returns
    -------
    float
        Approximate distance in kilometers.
    """
    d_lat = (lat2 - lat1) * 111.0
    d_lon = (lon2 - lon1) * 85.0
    return math.sqrt((d_lat * d_lat) + (d_lon * d_lon))


def _calculate_bearing(
    lat1: float,
    lon1: float,
    lat2: float,
    lon2: float
) -> int:
    """
    Calculate heading angle in degrees toward target.

    Parameters
    ----------
    lat1 : float
        Current latitude in degrees.
    lon1 : float
        Current longitude in degrees.
    lat2 : float
        Target latitude in degrees.
    lon2 : float
        Target longitude in degrees.

    Returns
    -------
    int
        Heading bearing in degrees (0 to 359).
    """
    d_lon = math.radians(lon2 - lon1)
    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    y_val = math.sin(d_lon) * math.cos(phi2)
    x_val = math.cos(phi1) * math.sin(phi2) - (
        math.sin(phi1) * math.cos(phi2) * math.cos(d_lon)
    )
    return int((math.degrees(math.atan2(y_val, x_val)) + 360.0) % 360)


def _format_coord(val: float, pos_c: str, neg_c: str) -> str:
    """
    Format a decimal coordinate with cardinal direction.

    Parameters
    ----------
    val : float
        Coordinate value in degrees.
    pos_c : str
        Cardinal letter for positive values.
    neg_c : str
        Cardinal letter for negative values.

    Returns
    -------
    str
        Formatted coordinate string.
    """
    card = pos_c if val >= 0.0 else neg_c
    return f"{abs(val):.6f} deg {card}"


def _format_pos(lat: float, lon: float) -> str:
    """
    Format combined latitude and longitude string.

    Parameters
    ----------
    lat : float
        Latitude coordinate.
    lon : float
        Longitude coordinate.

    Returns
    -------
    str
        Formatted dual coordinate string.
    """
    lat_s = _format_coord(lat, 'N', 'S')
    lon_s = _format_coord(lon, 'E', 'W')
    return f"{lat_s}  {lon_s}"


def _format_hud_rows(
    cur_lat: float,
    cur_lon: float,
    tgt_lat: float,
    tgt_lon: float,
    is_rel: bool
) -> list[str]:
    """
    Format HUD data rows.

    Parameters
    ----------
    cur_lat : float
        Current UAV latitude.
    cur_lon : float
        Current UAV longitude.
    tgt_lat : float
        Target waypoint latitude.
    tgt_lon : float
        Target waypoint longitude.
    is_rel : bool
        Whether payload has released.

    Returns
    -------
    list[str]
        List of formatted box rows.
    """
    dist = _calculate_distance(cur_lat, cur_lon, tgt_lat, tgt_lon)
    brg = _calculate_bearing(cur_lat, cur_lon, tgt_lat, tgt_lon)
    c_s = _format_pos(cur_lat, cur_lon)
    t_s = _format_pos(tgt_lat, tgt_lon)
    r_s = f"{dist:>7.2f} km / {brg:>03d} deg"
    l_s = "RELEASED [GPIO16 HIGH]" if is_rel else "SECURED [GPIO16 LOW]"
    return [
        f"| CURRENT POSITION : {c_s:<44} |",
        f"| TARGET WAYPOINT  : {t_s:<44} |",
        f"| RANGE / BEARING  : {r_s:<44} |",
        f"| PAYLOAD LATCH    : {l_s:<44} |"
    ]


def _print_box(title: str, link: str, rows: list[str]) -> None:
    """
    Print framed ASCII box.

    Parameters
    ----------
    title : str
        Box title line.
    link : str
        Sub-header line.
    rows : list[str]
        Body content rows.

    Returns
    -------
    None
    """
    hdr = f"+{'-' * 65}+"
    print(hdr)
    print(title)
    print(link)
    print(hdr)
    for r in rows:
        print(r)
    print(hdr + "\n")


def _print_hud(
    cur_lat: float,
    cur_lon: float,
    tgt_lat: float,
    tgt_lon: float,
    is_released: bool
) -> None:
    """
    Display the 67-character telemetry mission HUD.

    Parameters
    ----------
    cur_lat : float
        Current UAV latitude.
    cur_lon : float
        Current UAV longitude.
    tgt_lat : float
        Target waypoint latitude.
    tgt_lon : float
        Target waypoint longitude.
    is_released : bool
        Whether payload solenoid has been energized.

    Returns
    -------
    None
    """
    rows = _format_hud_rows(cur_lat, cur_lon, tgt_lat, tgt_lon, is_released)
    title = f"|{'DARK VECTOR TELEMETRY CONSOLE':^65}|"
    status = f"{'STATUS: ONLINE':>20}"
    link = f"| LINK: FT232RL / RYLR998 LORA GROUND STATION{status} |"
    _print_box(title, link, rows)


def _run_demo() -> None:
    """
    Execute simulation of drone telemetry stream.

    Parameters
    ----------
    None

    Returns
    -------
    None
    """
    print("[*] Running simulated ground station telemetry stream...\n")
    _print_hud(38.840280, -77.428890, 38.881940, -77.450280, False)
    time.sleep(1.0)
    _print_hud(38.861110, -77.439585, 38.881940, -77.450280, False)
    time.sleep(1.0)
    _print_hud(38.881940, -77.450280, 38.881940, -77.450280, True)


def _process_line(
    line: str,
    coords: dict[str, float]
) -> None:
    """
    Parse a single line of serial telemetry.

    Parameters
    ----------
    line : str
        Raw serial string.
    coords : dict[str, float]
        State dictionary of coordinates.

    Returns
    -------
    None
    """
    if "CURRENT LAT:" in line and "LON:" in line:
        parts = line[line.index("CURRENT LAT:"):].replace(",", "").split()
        coords["cur_lat"] = float(parts[2])
        coords["cur_lon"] = float(parts[4])
    elif "TARGET LAT:" in line and "LON:" in line:
        parts = line[line.index("TARGET LAT:"):].replace(",", "").split()
        coords["tgt_lat"] = float(parts[2])
        coords["tgt_lon"] = float(parts[4])
    elif "PAYLOAD RELEASED" in line:
        coords["released"] = 1.0


def _monitor_serial(port: str, baud: int) -> None:
    """
    Monitor serial stream from FT232RL ground station.

    Parameters
    ----------
    port : str
        Serial device path.
    baud : int
        Baud rate.

    Returns
    -------
    None
    """
    if serial is None:
        sys.exit("pyserial required: pip install pyserial")
    coords = {"cur_lat": 0.0, "cur_lon": 0.0,
              "tgt_lat": 38.881940, "tgt_lon": -77.450280, "released": 0.0}
    with serial.Serial(port, baud, timeout=2.0) as ser:
        while True:
            raw = ser.readline().decode("utf-8", errors="ignore").strip()
            _process_line(raw, coords)
            rel = coords["released"] > 0.5
            _print_hud(coords["cur_lat"], coords["cur_lon"],
                       coords["tgt_lat"], coords["tgt_lon"], rel)


def main() -> None:
    """
    Parse arguments and start telemetry monitor.

    Parameters
    ----------
    None

    Returns
    -------
    None
    """
    parser = argparse.ArgumentParser(description="Dark Vector Telemetry")
    parser.add_argument("--port", default="/dev/tty.usbserial-0001")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--demo", action="store_true")
    args = parser.parse_args()
    if args.demo:
        _run_demo()
        return
    _monitor_serial(args.port, args.baud)


if __name__ == "__main__":
    main()

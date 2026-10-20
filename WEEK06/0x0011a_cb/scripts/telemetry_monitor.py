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
import re
import sys
import time

try:
    import serial
except ImportError:
    serial = None


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
    is_rel: bool,
    has_lock: bool = False
) -> list[str]:
    """
    Format HUD data rows.

    Parameters
    ----------
    cur_lat : float
        Current UAV latitude.
    cur_lon : float
        Current UAV longitude.
    is_rel : bool
        Whether payload has released.
    has_lock : bool
        Whether active GNSS 3D lock has been acquired.

    Returns
    -------
    list[str]
        List of formatted box rows.
    """
    if cur_lat == 0.0 and cur_lon == 0.0:
        c_s = "0.000000 deg N  0.000000 deg E"
        g_s = "SEARCHING SATELLITES"
        m_s = "MOTOR STOPPED [WAITING FOR 3D LOCK]"
    else:
        c_s = _format_pos(cur_lat, cur_lon)
        g_s = "ACTIVE 3D LOCK" if has_lock else "SEARCHING SATELLITES"
        m_s = "ACTIVE PROPULSION [SERVO SPINNING]" if has_lock else "MOTOR STOPPED [WAITING FOR 3D LOCK]"
    return [
        f"| CURRENT POSITION : {c_s:<44} |",
        f"| GNSS SUBSYSTEM   : {g_s:<44} |",
        f"| PROPULSION MOTOR : {m_s:<44} |"
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
    print(hdr + "\n", flush=True)


def _print_hud(
    cur_lat: float,
    cur_lon: float,
    is_released: bool,
    has_lock: bool = False
) -> None:
    """
    Display the 67-character telemetry mission HUD.

    Parameters
    ----------
    cur_lat : float
        Current UAV latitude.
    cur_lon : float
        Current UAV longitude.
    is_released : bool
        Whether payload solenoid has been energized.
    has_lock : bool
        Whether active GNSS 3D lock has been acquired.

    Returns
    -------
    None
    """
    rows = _format_hud_rows(cur_lat, cur_lon, is_released, has_lock)
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
    _print_hud(0.0, 0.0, False, False)
    time.sleep(1.0)
    _print_hud(38.840280, -77.428890, False, True)
    time.sleep(1.0)
    _print_hud(38.861110, -77.439585, False, True)
    time.sleep(1.0)
    _print_hud(38.881940, -77.450280, True, True)


def _process_line(
    line: str,
    coords: dict[str, any]
) -> bool:
    """
    Parse a single line of serial telemetry using regex.

    Parameters
    ----------
    line : str
        Raw serial string.
    coords : dict[str, any]
        State dictionary of coordinates.

    Returns
    -------
    bool
        True if telemetry data was updated, False otherwise.
    """
    updated = False
    m_cur = re.search(r"CURRENT LAT:\s*([-+]?\d*\.?\d+).*?LON:\s*([-+]?\d*\.?\d+)", line)
    if m_cur:
        coords["cur_lat"] = float(m_cur.group(1))
        coords["cur_lon"] = float(m_cur.group(2))
        coords["has_lock"] = True
        updated = True
    if "PAYLOAD RELEASED" in line:
        coords["released"] = 1.0
        updated = True
    return updated


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
    coords = {
        "cur_lat": 0.0,
        "cur_lon": 0.0,
        "released": 0.0,
        "has_lock": False
    }
    with serial.Serial(port, baud, timeout=1.0) as ser:
        while True:
            raw = ser.readline().decode("utf-8", errors="ignore").strip()
            if raw:
                _process_line(raw, coords)
            rel = coords["released"] > 0.5
            _print_hud(coords["cur_lat"], coords["cur_lon"], rel, coords["has_lock"])


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

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
# File:    decode_coordinates.py
# Desc:    Decode and patch RP2350 micro-UAV navigation coordinates.
# Created: 2026

"""
Decode and patch RP2350 micro-UAV navigation coordinates.

Analyzes raw firmware images for IEEE 754 64-bit double-precision floating
point coordinates. Scans target flight vectors and patches binaries with
safe disposal coordinates in the Atlantic Ocean.
"""

import math
import struct
import sys
from pathlib import Path

FLASH_BASE = 0x10000000
ORIGIN_LAT = 38.840280
ORIGIN_LON = -77.428890
ATLANTIC_LAT = 37.000000
ATLANTIC_LON = -74.000000


def _haversine_calc(phi1: float, phi2: float,
                    dphi: float, dlam: float) -> float:
    """
    Compute central angle using haversine formula.

    Parameters
    ----------
    phi1 : float
        Origin latitude in radians.
    phi2 : float
        Target latitude in radians.
    dphi : float
        Latitude difference in radians.
    dlam : float
        Longitude difference in radians.

    Returns
    -------
    float
        Central angular distance in radians.
    """
    s_phi = math.sin(dphi / 2.0) ** 2
    s_lam = math.sin(dlam / 2.0) ** 2
    a = s_phi + math.cos(phi1) * math.cos(phi2) * s_lam
    return 2.0 * math.atan2(math.sqrt(a), math.sqrt(1.0 - a))


def haversine(lat1: float, lon1: float,
              lat2: float, lon2: float) -> tuple[float, float]:
    """
    Compute Great-Circle distance and azimuth bearing.

    Parameters
    ----------
    lat1 : float
        Origin latitude in degrees.
    lon1 : float
        Origin longitude in degrees.
    lat2 : float
        Destination latitude in degrees.
    lon2 : float
        Destination longitude in degrees.

    Returns
    -------
    tuple[float, float]
        Distance in statute miles and bearing in degrees.
    """
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dl = math.radians(lon2 - lon1)
    dist = 6371.0 * _haversine_calc(p1, p2, math.radians(lat2 - lat1), dl)
    y = math.sin(dl) * math.cos(p2)
    term = math.sin(p1) * math.cos(p2) * math.cos(dl)
    x = math.cos(p1) * math.sin(p2) - term
    bearing = (math.degrees(math.atan2(y, x)) + 360.0) % 360.0
    return dist * 0.621371, bearing


def _is_coord(val: float) -> bool:
    """
    Verify if float falls within target geographic bounds.

    Parameters
    ----------
    val : float
        Candidate double-precision value.

    Returns
    -------
    bool
        True if value is a valid latitude or longitude.
    """
    if math.isnan(val) or math.isinf(val):
        return False
    in_lat = 35.0 <= val <= 41.0
    in_lon = -79.0 <= val <= -72.0
    return in_lat or in_lon


def _parse_chunk(data: bytes, off: int) -> dict | None:
    """
    Extract and validate one 8-byte candidate float.

    Parameters
    ----------
    data : bytes
        Firmware image buffer.
    off : int
        Byte offset inside image buffer.

    Returns
    -------
    dict | None
        Parsed coordinate record or None.
    """
    chunk = data[off:off + 8]
    val = struct.unpack("<d", chunk)[0]
    if not _is_coord(val):
        return None
    hex_str = " ".join(f"{b:02x}" for b in chunk)
    u64 = struct.unpack("<Q", chunk)[0]
    kind = "LATITUDE" if val > 0.0 else "LONGITUDE"
    return {"off": off, "addr": FLASH_BASE + off, "val": val,
            "hex": hex_str, "u64": u64, "kind": kind}


def scan_coordinates(data: bytes) -> list[dict]:
    """
    Scan firmware buffer for double-precision coordinates.

    Parameters
    ----------
    data : bytes
        Firmware image buffer.

    Returns
    -------
    list[dict]
        List of candidate coordinate records.
    """
    found = []
    limit = len(data) - 8
    for off in range(0, limit, 4):
        item = _parse_chunk(data, off)
        if item is not None:
            found.append(item)
    return found


def _print_banner(path: Path) -> None:
    """
    Print operation heading and recovery origin.

    Parameters
    ----------
    path : pathlib.Path
        Target firmware path.

    Returns
    -------
    None
    """
    print("=" * 67)
    print("  OPERATION DARK VECTOR // FORENSIC COORDINATE TOOL")
    print("  GMU Rapid Hardware Exploitation Laboratory - Fairfax, VA")
    print("=" * 67)
    print(f"[*] Target Binary: {path.name} ({path.stat().st_size:,} bytes)")
    print(f"[*] Recovery Origin: Centreville, VA "
          f"({ORIGIN_LAT:.6f}, {ORIGIN_LON:.6f})")
    print("-" * 67)


def _print_candidate(c: dict) -> None:
    """
    Print formatted candidate coordinate entry.

    Parameters
    ----------
    c : dict
        Candidate coordinate record.

    Returns
    -------
    None
    """
    print(f"  [{c['kind']:9s}] Value: {c['val']:12.6f} | "
          f"Addr: 0x{c['addr']:08x} (Offset: 0x{c['off']:04x})")
    print(f"               Hex:   {c['hex']} | uint64: 0x{c['u64']:016x}")


def _cardinal_bearing(brg: float) -> str:
    """Return compass direction string for given bearing."""
    dirs = ["N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
            "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"]
    idx = int((brg + 11.25) / 22.5) % 16
    return dirs[idx]


def _print_summary(lat: float, lon: float, mi: float, brg: float) -> None:
    """
    Print mission tactical assessment summary.

    Parameters
    ----------
    lat : float
        Decoded target latitude.
    lon : float
        Decoded target longitude.
    mi : float
        Distance in statute miles.
    brg : float
        Initial bearing in degrees.

    Returns
    -------
    None
    """
    card = _cardinal_bearing(brg)
    print("\n" + "=" * 67)
    print("  TACTICAL MISSION PROFILE DECODED")
    print("=" * 67)
    print(f"  Target Latitude:       {lat:.6f} deg N")
    print(f"  Target Longitude:      {lon:.6f} deg W")
    print(f"  Distance from Origin:  {mi:.2f} miles ({mi * 1.60934:.2f} km)")
    print(f"  Flight Vector Bearing: {brg:.1f} deg ({card})")
    print("=" * 67)


def _patch_bytes(data: bytes, old_lat: float, old_lon: float) -> bytes:
    """
    Replace target coordinates with safe Atlantic Ocean coordinates.

    Parameters
    ----------
    data : bytes
        Original firmware bytes.
    old_lat : float
        Original target latitude.
    old_lon : float
        Original target longitude.

    Returns
    -------
    bytes
        Patched firmware byte buffer.
    """
    src_lat = struct.pack("<d", old_lat)
    src_lon = struct.pack("<d", old_lon)
    dst_lat = struct.pack("<d", ATLANTIC_LAT)
    dst_lon = struct.pack("<d", ATLANTIC_LON)
    buf = data.replace(src_lat, dst_lat)
    return buf.replace(src_lon, dst_lon)


def _print_patch_info(out_name: str, mi: float, brg: float) -> None:
    """
    Display confirmation of applied firmware patch.

    Parameters
    ----------
    out_name : str
        Patched output file name.
    mi : float
        Distance to safe disposal zone.
    brg : float
        Azimuth bearing to disposal zone.

    Returns
    -------
    None
    """
    print(f"\n[+] Patched firmware written to: {out_name}")
    print("[+] Overwrote waypoint -> Atlantic Ocean Disposal Zone:")
    print(f"    Safe Latitude:  {ATLANTIC_LAT:.6f} deg N")
    print(f"    Safe Longitude: {ATLANTIC_LON:.6f} deg W")
    print(f"    Offshore Distance: {mi:.2f} miles (Bearing: {brg:.1f} deg)")


def patch_firmware(target: Path, out_path: Path,
                   lat: float, lon: float) -> None:
    """
    Patch firmware with safe Atlantic Ocean disposal waypoint.

    Parameters
    ----------
    target : pathlib.Path
        Source binary path.
    out_path : pathlib.Path
        Destination patched binary path.
    lat : float
        Current target latitude.
    lon : float
        Current target longitude.

    Returns
    -------
    None
    """
    raw = target.read_bytes()
    patched = _patch_bytes(raw, lat, lon)
    out_path.write_bytes(patched)
    mi, brg = haversine(ORIGIN_LAT, ORIGIN_LON, ATLANTIC_LAT, ATLANTIC_LON)
    _print_patch_info(out_path.name, mi, brg)


def _evaluate(items: list[dict], target: Path, do_patch: bool) -> None:
    """
    Display results and execute patch if requested.

    Parameters
    ----------
    items : list[dict]
        Found coordinate records.
    target : pathlib.Path
        Target binary path.
    do_patch : bool
        Flag indicating if patch should be applied.

    Returns
    -------
    None
    """
    lats = [c for c in items if c["kind"] == "LATITUDE"]
    lons = [c for c in items if c["kind"] == "LONGITUDE"]
    if not (lats and lons):
        return
    t_lat, t_lon = lats[-1]["val"], lons[-1]["val"]
    mi, brg = haversine(ORIGIN_LAT, ORIGIN_LON, t_lat, t_lon)
    _print_summary(t_lat, t_lon, mi, brg)
    if do_patch:
        out = target.parent / f"{target.stem}_patched.bin"
        patch_firmware(target, out, t_lat, t_lon)


def _parse_args(args: list[str]) -> tuple[Path, bool]:
    """
    Parse command line arguments for target path and patch flag.

    Parameters
    ----------
    args : list[str]
        Command line arguments.

    Returns
    -------
    tuple[pathlib.Path, bool]
        Target binary path and patch flag.
    """
    do_patch = "--patch" in args
    paths = [p for p in args if not p.startswith("--")]
    target = Path(paths[0]) if paths else Path("0x0011a_cb.bin")
    return target, do_patch


def main() -> int:
    """
    Execute firmware coordinate extraction and optional patching.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success, non-zero on failure.
    """
    target, do_patch = _parse_args(sys.argv[1:])
    if not target.exists():
        print(f"[-] Error: '{target}' not found.")
        return 1
    _print_banner(target)
    items = scan_coordinates(target.read_bytes())
    for item in items:
        _print_candidate(item)
    _evaluate(items, target, do_patch)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

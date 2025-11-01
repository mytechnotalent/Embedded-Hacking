#!/usr/bin/env python3

"""
FILE: safe_patch.py

DESCRIPTION:
Incremental firmware patcher with minimal, conservative steps to bias readings.

BRIEF:
Provides three modes:
- pool_only: change only the literal pool constant (scaling effect; least invasive)
- temp_only_vadd: convert temperature path to vadd.f32 and set a small negative pool
- both_vadd: convert both paths to vadd.f32 and set a small negative pool

Intended to let you test changes incrementally, keeping humidity stable while
you evaluate temperature bias first.

AUTHOR: Kevin Thomas
CREATION DATE: November 1, 2025
UPDATE DATE: November 1, 2025
"""

import sys
import os
import struct

BIN_PATH = "build/0x001a_operators.bin"

# Offsets
OFF_410 = 0x410
OFF_414 = 0x414
OFF_POOL = 0x42C

# Original encodings (for visibility only)
ORIG_410 = bytes.fromhex("a6ee257a")  # vfma.f32 s14, s12, s11
ORIG_414 = bytes.fromhex("e6eea57a")  # vfma.f32 s15, s13, s11
ORIG_POOL = bytes.fromhex("cccccc3d")  # 0.1f

# Encodings for vadd.f32 s14, s14, s11 and vadd.f32 s15, s15, s11
VADD_S14_S11 = struct.pack("<I", 0xEEB47A06)  # 06 7a b4 ee
VADD_S15_S11 = struct.pack("<I", 0xEEB47A86)  # 86 7a b4 ee


def read_at(path, offset, n):
    """Read bytes from a binary file at a given offset.

    Parameters
    ----------
    path : str
        Filesystem path to the binary file.
    offset : int
        Byte offset from the start of the file.
    n : int
        Number of bytes to read.

    Returns
    -------
    bytes
        The raw bytes read from the file.

    Raises
    ------
    IOError
        If the file cannot be opened or read.
    """
    with open(path, "rb") as f:
        f.seek(offset)
        return f.read(n)


def write_at(path, offset, data):
    """Write bytes into a binary file at a given offset.

    Parameters
    ----------
    path : str
        Filesystem path to the binary file.
    offset : int
        Byte offset from the start of the file.
    data : bytes
        Bytes to write into the file.

    Raises
    ------
    IOError
        If the file cannot be opened or written.
    """
    with open(path, "rb+") as f:
        f.seek(offset)
        f.write(data)


def fmt_float(b):
    """Format a 4-byte sequence as hex and float if possible.

    Parameters
    ----------
    b : bytes
        A 4-byte sequence.

    Returns
    -------
    str
        Hexadecimal string with float interpretation if valid.
    """
    hx = b.hex()
    try:
        val = struct.unpack("<f", b)[0]
        return f"{hx} (float {val:.6f})"
    except Exception:
        return hx


def patch_pool_only(new_pool_float=-1.0):
    """Patch only the pool constant to a new float value.

    Parameters
    ----------
    new_pool_float : float, optional
        New float value to write into the pool constant (default -1.0).

    Returns
    -------
    None
    """
    print("[mode] pool_only")
    curp = read_at(BIN_PATH, OFF_POOL, 4)
    print(f"Current @0x1000042C: {fmt_float(curp)} (expected {ORIG_POOL.hex()} == 0.1f)")
    newb = struct.pack("<f", new_pool_float)
    write_at(BIN_PATH, OFF_POOL, newb)
    chkp = read_at(BIN_PATH, OFF_POOL, 4)
    print(f"Patched @0x1000042C: {fmt_float(chkp)} (target {newb.hex()} == {new_pool_float}f)")


def patch_temp_only_vadd(new_pool_float=-2.0):
    """Patch temperature path to vadd and set pool to a small negative.

    Parameters
    ----------
    new_pool_float : float, optional
        New float value to write into the pool constant (default -2.0).

    Returns
    -------
    None
    """
    print("[mode] temp_only_vadd")
    cur414 = read_at(BIN_PATH, OFF_414, 4)
    curp = read_at(BIN_PATH, OFF_POOL, 4)
    print(f"Current @0x10000414: {cur414.hex()} (expected {ORIG_414.hex()})")
    print(f"Current @0x1000042C: {fmt_float(curp)}")
    write_at(BIN_PATH, OFF_414, VADD_S15_S11)
    write_at(BIN_PATH, OFF_POOL, struct.pack("<f", new_pool_float))
    chk414 = read_at(BIN_PATH, OFF_414, 4)
    chkp = read_at(BIN_PATH, OFF_POOL, 4)
    print(f"Patched @0x10000414: {chk414.hex()} (should be {VADD_S15_S11.hex()})")
    print(f"Patched @0x1000042C: {fmt_float(chkp)} (target {new_pool_float}f)")


def patch_both_vadd(new_pool_float=-2.0):
    """Patch both humidity and temperature paths to vadd and set pool.

    Parameters
    ----------
    new_pool_float : float, optional
        New float value to write into the pool constant (default -2.0).

    Returns
    -------
    None
    """
    print("[mode] both_vadd")
    cur410 = read_at(BIN_PATH, OFF_410, 4)
    cur414 = read_at(BIN_PATH, OFF_414, 4)
    curp = read_at(BIN_PATH, OFF_POOL, 4)
    print(f"Current @0x10000410: {cur410.hex()} (expected {ORIG_410.hex()})")
    print(f"Current @0x10000414: {cur414.hex()} (expected {ORIG_414.hex()})")
    print(f"Current @0x1000042C: {fmt_float(curp)}")
    write_at(BIN_PATH, OFF_410, VADD_S14_S11)
    write_at(BIN_PATH, OFF_414, VADD_S15_S11)
    write_at(BIN_PATH, OFF_POOL, struct.pack("<f", new_pool_float))
    chk410 = read_at(BIN_PATH, OFF_410, 4)
    chk414 = read_at(BIN_PATH, OFF_414, 4)
    chkp = read_at(BIN_PATH, OFF_POOL, 4)
    print(f"Patched @0x10000410: {chk410.hex()} (should be {VADD_S14_S11.hex()})")
    print(f"Patched @0x10000414: {chk414.hex()} (should be {VADD_S15_S11.hex()})")
    print(f"Patched @0x1000042C: {fmt_float(chkp)} (target {new_pool_float}f)")


def main():
    """Dispatch patch mode and perform incremental modifications.

    Returns
    -------
    None

    Raises
    ------
    FileNotFoundError
        If the binary file specified by BIN_PATH does not exist.
    """
    if not os.path.exists(BIN_PATH):
        raise FileNotFoundError(f"Binary not found: {BIN_PATH}")
    mode = (sys.argv[1] if len(sys.argv) > 1 else "pool_only").strip().lower()
    if mode == "pool_only":
        patch_pool_only(new_pool_float=-1.0)
    elif mode == "temp_only_vadd":
        patch_temp_only_vadd(new_pool_float=-2.0)
    elif mode == "both_vadd":
        patch_both_vadd(new_pool_float=-2.0)
    else:
        print(f"Unknown mode: {mode}")
        print("Use: pool_only | temp_only_vadd | both_vadd")
        sys.exit(2)


if __name__ == "__main__":
    main()
    print("Patch complete. Convert to UF2 and flash. Test each mode incrementally.")

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
# File:    patch_binary.py
# Desc:    Patch script for Operation Zero Hour (0x0014a_cb).
# Created: 2026

"""
Patch script for Operation Zero Hour (0x0014a_cb).

Patches the conditional check in 0x0014a_cb.bin to force the WARHEAD DISARMED
// SAFE state on boot.
"""

import sys

PATCH_OFFSET = 0x688
EXPECTED_OPCODE = b"\x03\x2c"
PATCHED_BYTE = 0x01


def _apply_patch(data: bytearray) -> None:
    """
    Verify opcode and apply single-byte conditional branch patch.

    Parameters
    ----------
    data : bytearray
        Mutable binary image buffer.

    Returns
    -------
    None
    """
    if data[PATCH_OFFSET:PATCH_OFFSET + 2] != EXPECTED_OPCODE:
        actual = data[PATCH_OFFSET:PATCH_OFFSET + 2].hex()
        raise ValueError(f"Expected 03 2c at offset 0x{PATCH_OFFSET:x}, got {actual}")
    data[PATCH_OFFSET] = PATCHED_BYTE


def patch_binary(in_bin: str, out_bin: str) -> None:
    """
    Read target firmware binary, apply patch, and write modified image.

    Parameters
    ----------
    in_bin : str
        Path to source binary file.
    out_bin : str
        Path to destination patched binary file.

    Returns
    -------
    None
    """
    with open(in_bin, "rb") as f:
        data = bytearray(f.read())
    _apply_patch(data)
    with open(out_bin, "wb") as f:
        f.write(data)
    print(f"[+] Successfully patched {in_bin} -> {out_bin} at offset 0x{PATCH_OFFSET:x}")


def main() -> None:
    """
    Parse command line arguments and execute binary patching.

    Parameters
    ----------
    None

    Returns
    -------
    None
    """
    in_file = sys.argv[1] if len(sys.argv) > 1 else "0x0014a_cb.bin"
    out_file = sys.argv[2] if len(sys.argv) > 2 else "0x0014a_cb_patched.bin"
    patch_binary(in_file, out_file)


if __name__ == "__main__":
    main()

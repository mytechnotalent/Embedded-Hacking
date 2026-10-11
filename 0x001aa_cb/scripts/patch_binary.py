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
# Desc:    Patch script for Operation Iron Net (0x001aa_cb).
# Created: 2026

"""
Patch script for Operation Iron Net (0x001aa_cb).

Forces the environment gate and the authorization-token gate to true so the
node deploys the payload on boot without a valid sensor reading or token.
"""

import sys

PATCHES = [
    (0x7BC, b"\x58\xd1", b"\x00\xbf", "nop (force authorization accept)"),
    (0x802, b"\x1b\xb1", b"\x00\xbf", "nop (ignore the environment gate)"),
    (0x7D2, b"\x02\x2b", b"\xff\x2b", "cmp r3, #0xff (raise fault threshold)"),
]


def _apply_patch(data: bytearray) -> None:
    """
    Verify each opcode and apply its replacement bytes in place.

    Parameters
    ----------
    data : bytearray
        Mutable binary image buffer.

    Returns
    -------
    None
    """
    for offset, expected, patched, note in PATCHES:
        if data[offset:offset + len(expected)] != expected:
            actual = data[offset:offset + len(expected)].hex()
            raise ValueError(f"Expected {expected.hex()} at 0x{offset:x}, got {actual}")
        data[offset:offset + len(patched)] = patched
        print(f"[+] Patched 0x{offset:x}: {note}")


def patch_binary(in_bin: str, out_bin: str) -> None:
    """
    Read target firmware binary, apply patches, and write modified image.

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
    print(f"[+] Successfully patched {in_bin} -> {out_bin}")


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
    in_file = sys.argv[1] if len(sys.argv) > 1 else "0x001aa_cb.bin"
    out_file = sys.argv[2] if len(sys.argv) > 2 else "0x001aa_cb_patched.bin"
    patch_binary(in_file, out_file)


if __name__ == "__main__":
    main()

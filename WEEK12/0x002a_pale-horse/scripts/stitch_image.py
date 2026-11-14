#!/usr/bin/env python3
"""Stitch the PALE HORSE flash image from the built stage binaries.

Lays the stage-2 loader, the padded application, the encrypted ghost blob and
the manifest sector into one image at 0x10000000 and records the CRC-32 of the
application in the manifest header.
"""
import struct
import sys
import zlib
from pathlib import Path

BASE = 0x10000000
APP_OFF = 0x1000
APP_SIZE = 0x4000
BLOB_OFF = 0x3E000
MAN_OFF = 0x3F000
TOTAL = MAN_OFF + 0x1000
COMPANY = b"FUN SOCIETY AMUSEMENT, LLC"


def padded(path, size):
    """
    Load a file padded with 0xFF to a fixed size.

    Parameters
    ----------
    path : pathlib.Path
        Source file.
    size : int
        Target size in bytes.

    Returns
    -------
    bytes
        Padded bytes.
    """
    return path.read_bytes()[:size].ljust(size, b"\xff")


def place(image, offset, data):
    """
    Copy data into the image at an offset.

    Parameters
    ----------
    image : bytearray
        Destination image.
    offset : int
        Destination offset.
    data : bytes
        Bytes to place.

    Returns
    -------
    None
    """
    image[offset:offset + len(data)] = data


def build(out):
    """
    Compose the image from the built parts.

    Parameters
    ----------
    out : pathlib.Path
        Build directory.

    Returns
    -------
    bytearray
        Combined image.
    """
    image = bytearray(b"\xff" * TOTAL)
    place(image, 0, (out / "stage2.bin").read_bytes())
    place(image, APP_OFF, padded(out / "app.bin", APP_SIZE))
    place(image, BLOB_OFF, padded(out / "ghost_blob.bin", 0x1000))
    return image


def seal(image):
    """
    Write the CRC and manifest into the image.

    Parameters
    ----------
    image : bytearray
        Combined image.

    Returns
    -------
    int
        Application CRC-32.
    """
    crc = zlib.crc32(bytes(image[APP_OFF:APP_OFF + APP_SIZE]))
    crc &= 0xFFFFFFFF
    struct.pack_into("<I", image, MAN_OFF, crc)
    payload = COMPANY + b":" + struct.pack("<I", crc) + b":7F3A"
    place(image, MAN_OFF + 4, payload)
    return crc


def main():
    """
    Stitch the image for a build directory.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    out = Path(sys.argv[1])
    image = build(out)
    crc = seal(image)
    (out / "pale-horse.bin").write_bytes(image)
    print(f"app CRC32 = 0x{crc:08x}; image = {len(image)} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

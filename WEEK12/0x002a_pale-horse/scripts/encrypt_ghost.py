#!/usr/bin/env python3
"""Encrypt the ghost stage for OPERATION PALE HORSE.

Mirrors stage2/decrypt.s: XOR each byte against an advancing keystream that
rotates left five bits and adds 0x9e, modulo 2**32. The first word is
XOR-masked with the ghost magic so the loader's runtime patch restores it.
"""
import sys
from pathlib import Path

GHOST_KEY = 0x50414C45
GHOST_MAGIC = 0x50414C45
GHOST_SIZE = 0x1000
MASK = 0xFFFFFFFF
STEP = 0x9E


def advance(key: int) -> int:
    """
    Advance the keystream by one byte.

    Parameters
    ----------
    key : int
        Current 32-bit keystream key.

    Returns
    -------
    int
        Next 32-bit keystream key.
    """
    rotated = ((key << 5) | (key >> 27)) & MASK
    return (rotated + STEP) & MASK


def keystream(data: bytes, key: int) -> bytes:
    """
    Transform data with the XOR keystream.

    Parameters
    ----------
    data : bytes
        Input bytes.
    key : int
        Initial 32-bit keystream key.

    Returns
    -------
    bytes
        Transformed bytes.
    """
    out = bytearray()
    for value in data:
        out.append(value ^ (key & 0xFF))
        key = advance(key)
    return bytes(out)


def masked(data: bytearray) -> bytearray:
    """
    XOR-mask the first word with the ghost magic.

    Parameters
    ----------
    data : bytearray
        Padded ghost image.

    Returns
    -------
    bytearray
        Image with the first word masked.
    """
    word = int.from_bytes(data[0:4], "little") ^ GHOST_MAGIC
    data[0:4] = word.to_bytes(4, "little")
    return data


def load(path: str) -> bytearray:
    """
    Load and pad the ghost image.

    Parameters
    ----------
    path : str
        Ghost binary path.

    Returns
    -------
    bytearray
        Padded ghost image.
    """
    raw = Path(path).read_bytes()[:GHOST_SIZE]
    return bytearray(raw.ljust(GHOST_SIZE, b"\xff"))


def main() -> int:
    """
    Encrypt one ghost image.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success, otherwise one.
    """
    if len(sys.argv) != 3:
        print("usage: encrypt_ghost.py <ghost.bin> <out.bin>", file=sys.stderr)
        return 1
    data = masked(load(sys.argv[1]))
    Path(sys.argv[2]).write_bytes(keystream(bytes(data), GHOST_KEY))
    print(f"encrypted {len(data)} bytes -> {sys.argv[2]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

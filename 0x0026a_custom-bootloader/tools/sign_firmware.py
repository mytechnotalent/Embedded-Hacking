#!/usr/bin/env python3
"""Sign a Raspberry Pi Pico application image for the 0x0026a bootloader.

The signer reads the raw application binary (the bytes the bootloader will see
in XIP flash), computes an AES-128-CMAC tag over it, and writes the fixed-layout
manifest the bootloader expects into the sector at 0x1001F000. The AES and CMAC
routines are implemented here so the tool runs on a stock Python 3 install with
no third-party packages.

Examples
--------
Sign an image and write the manifest::

    python3 sign_firmware.py build/0x0026a_app.bin --out build/manifest.bin

Verify an existing manifest against an image::

    python3 sign_firmware.py build/0x0026a_app.bin --verify build/manifest.bin
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

MANIFEST_MAGIC = 0x424C4657
MANIFEST_VERSION = 1
MANIFEST_SECTOR_SIZE = 4096
DEFAULT_APP_BASE = 0x10020000
DEFAULT_KEY = b"IronNetBootKey!!"
RB = 0x1B
RB_128 = 0x87

SBOX = (
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
    0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
    0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
    0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
    0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
    0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
    0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
    0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
    0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
    0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
    0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
    0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
    0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
    0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
    0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
    0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
    0xB0, 0x54, 0xBB, 0x16,
)

RCON = (0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36)


def _xtime(value: int) -> int:
    """Multiply a byte by x in GF(2^8)."""
    value <<= 1
    return (value ^ RB) & 0xFF if value & 0x100 else value


def _expand_key(key: bytes) -> list[int]:
    """Expand a 16-byte key into the 176-byte AES-128 round key schedule."""
    schedule = list(key)
    for index in range(16, 176, 4):
        word = schedule[index - 4:index]
        if index % 16 == 0:
            word = word[1:] + word[:1]
            word = [SBOX[byte] for byte in word]
            word[0] ^= RCON[index // 16 - 1]
        for offset in range(4):
            schedule.append(schedule[index - 16 + offset] ^ word[offset])
    return schedule


def _add_round_key(state: list[int], schedule: list[int]) -> None:
    """XOR the round key into the state in place."""
    for index in range(16):
        state[index] ^= schedule[index]


def _sub_bytes(state: list[int]) -> None:
    """Apply the AES S-box to every byte of the state."""
    for index in range(16):
        state[index] = SBOX[state[index]]


def _shift_rows(state: list[int]) -> None:
    """Rotate each row of the AES state left by its row index."""
    for row in range(1, 4):
        cells = [state[row + 4 * col] for col in range(4)]
        cells = cells[row:] + cells[:row]
        for col in range(4):
            state[row + 4 * col] = cells[col]


def _mix_columns(state: list[int]) -> None:
    """Mix each column of the AES state with the fixed AES matrix."""
    for col in range(4):
        base = 4 * col
        a0, a1, a2, a3 = state[base:base + 4]
        total = a0 ^ a1 ^ a2 ^ a3
        state[base + 0] = a0 ^ total ^ _xtime(a0 ^ a1)
        state[base + 1] = a1 ^ total ^ _xtime(a1 ^ a2)
        state[base + 2] = a2 ^ total ^ _xtime(a2 ^ a3)
        state[base + 3] = a3 ^ total ^ _xtime(a3 ^ a0)


def aes_encrypt_block(block: bytes, schedule: list[int]) -> bytes:
    """Encrypt one 16-byte block with the supplied round key schedule."""
    state = list(block)
    _add_round_key(state, schedule)
    for round_index in range(1, 10):
        _sub_bytes(state)
        _shift_rows(state)
        _mix_columns(state)
        _add_round_key(state, schedule[16 * round_index:])
    _sub_bytes(state)
    _shift_rows(state)
    _add_round_key(state, schedule[160:])
    return bytes(state)


def _double(block: bytes) -> bytes:
    """Left-shift a 128-bit big-endian value, folding in the CMAC constant."""
    value = int.from_bytes(block, "big")
    value <<= 1
    if block[0] & 0x80:
        value ^= RB_128
    return (value & ((1 << 128) - 1)).to_bytes(16, "big")


def _subkeys(schedule: list[int]) -> tuple[bytes, bytes]:
    """Derive the RFC 4493 K1 and K2 subkeys."""
    k1 = _double(aes_encrypt_block(bytes(16), schedule))
    return k1, _double(k1)


def _xor(left: bytes, right: bytes) -> bytes:
    """XOR two equal-length byte strings."""
    return bytes(a ^ b for a, b in zip(left, right))


def aes128_cmac(key: bytes, message: bytes) -> bytes:
    """Return the RFC 4493 AES-128-CMAC tag of a message."""
    schedule = _expand_key(key)
    k1, k2 = _subkeys(schedule)
    blocks = [message[i:i + 16] for i in range(0, len(message), 16)]
    complete = bool(blocks) and len(blocks[-1]) == 16
    if not complete:
        tail = (blocks[-1] + b"\x80") if blocks else b"\x80"
        blocks = blocks[:-1] + [tail.ljust(16, b"\x00")]
    last = blocks[-1]
    last = _xor(last, k1 if complete else k2)
    state = bytes(16)
    for block in blocks[:-1]:
        state = aes_encrypt_block(_xor(state, block), schedule)
    return aes_encrypt_block(_xor(state, last), schedule)


def build_manifest(app: bytes, app_base: int, key: bytes) -> bytes:
    """Assemble the padded manifest sector for an application image."""
    tag = aes128_cmac(key, app)
    header = struct.pack("<IIII", MANIFEST_MAGIC, MANIFEST_VERSION, app_base,
                         len(app)) + tag
    return header.ljust(MANIFEST_SECTOR_SIZE, b"\xFF")


def _parse_key(text: str) -> bytes:
    """Decode a hex or UTF-8 key argument into 16 bytes."""
    raw = bytes.fromhex(text) if len(text) == 32 else text.encode()
    if len(raw) != 16:
        raise argparse.ArgumentTypeError("key must be 16 bytes")
    return raw


def _read_image(path: str) -> bytes:
    """Read an application binary from disk."""
    data = Path(path).read_bytes()
    if not data:
        raise SystemExit(f"error: {path} is empty")
    return data


def _cmd_sign(args: argparse.Namespace) -> int:
    """Sign an image and write the manifest sector."""
    app = _read_image(args.image)
    manifest = build_manifest(app, args.app_base, args.key)
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(manifest)
    tag = manifest[16:32].hex()
    print(f"image      : {args.image} ({len(app)} bytes)")
    print(f"app_base   : 0x{args.app_base:08x}")
    print(f"cmac tag   : {tag}")
    print(f"manifest   : {out} ({len(manifest)} bytes)")
    return 0


def _cmd_verify(args: argparse.Namespace) -> int:
    """Recompute the tag and compare it against a manifest on disk."""
    app = _read_image(args.image)
    manifest = Path(args.verify).read_bytes()
    stored = manifest[16:32]
    expected = aes128_cmac(args.key, app)
    print(f"stored   : {stored.hex()}")
    print(f"computed : {expected.hex()}")
    if stored != expected:
        print("result   : MISMATCH")
        return 1
    print("result   : MATCH")
    return 0


def main() -> int:
    """Parse arguments and dispatch to sign or verify."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", help="raw application .bin to sign")
    parser.add_argument("--out", default="manifest.bin",
                        help="manifest output path (default: manifest.bin)")
    parser.add_argument("--app-base", type=lambda v: int(v, 0),
                        default=DEFAULT_APP_BASE,
                        help="application base address (default: 0x10020000)")
    parser.add_argument("--key", type=_parse_key, default=DEFAULT_KEY,
                        help="16-byte AES key as text or 32 hex digits")
    parser.add_argument("--verify", metavar="MANIFEST",
                        help="verify an existing manifest instead of signing")
    args = parser.parse_args()
    if args.verify:
        return _cmd_verify(args)
    return _cmd_sign(args)


if __name__ == "__main__":
    raise SystemExit(main())

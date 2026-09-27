#!/usr/bin/env python3
"""Verify every technical claim of Operation Black Start against CTF-01.bin.

Exits 0 only when every address, byte, and hash in CTF-R.md and CTF-S.md
matches the shipped image.
"""
import hashlib
import struct
import sys
from pathlib import Path

BASE = 0x10000000
ROOT = Path(__file__).resolve().parent.parent
BIN = ROOT / "CTF-01.bin"
UF2 = ROOT / "CTF-01.uf2"

EXPECTED_BIN_SHA = "6fd296f7a85f243fb26bf6bffcbeab26815fd8915101a81f72069063a5635e5a"
EXPECTED_UF2_SHA = "980f04369c23ad32a063dfe18de5af08df3830138fc7e7898b1f011b4f5e1d9d"

CMP_A = 0x100001FC
CMP_B = 0x1000020A
STRING_SIGNAL = 0x10003678
STRING_FRAME = 0x100037A0
LOOP_BACK = 0x10000266

RESULTS = []


def check(label, ok, detail=""):
    """Record one verification result.

    Parameters
    ----------
    label : str
        Human-readable check name.
    ok : bool
        Whether the check passed.
    detail : str
        Extra context printed with the result.

    Returns
    -------
    None
    """
    RESULTS.append(ok)
    print(f"[{'PASS' if ok else 'FAIL'}] {label} {detail}")


def main():
    """Run all verification checks.

    Returns
    -------
    int
        Zero when every check passes, else one.
    """
    data = BIN.read_bytes()
    check("CTF-01.bin SHA-256", hashlib.sha256(data).hexdigest() == EXPECTED_BIN_SHA)
    check("CTF-01.uf2 SHA-256",
          hashlib.sha256(UF2.read_bytes()).hexdigest() == EXPECTED_UF2_SHA)
    check("vector table", data[0:32].hex() ==
          "002008205b0100101b0100101d01001011010010110100101101001011010010")
    check("initial SP", struct.unpack("<I", data[0:4])[0] == 0x20082000)
    check("reset vector", struct.unpack("<I", data[4:8])[0] == 0x1000015B)
    check("compare site A", data[CMP_A - BASE:CMP_A - BASE + 2]
          == bytes.fromhex("5E2B"), "(cmp r3,#94)")
    check("compare site B", data[CMP_B - BASE:CMP_B - BASE + 2]
          == bytes.fromhex("5E2B"), "(cmp r3,#94)")
    check("loop back-edge", data[LOOP_BACK - BASE:LOOP_BACK - BASE + 2]
          == bytes.fromhex("E5E7"), "(b.n 0x10000234)")
    check("SIGNAL banner", data[STRING_SIGNAL - BASE:STRING_SIGNAL - BASE + 15]
          == b"SIGNAL: NORMAL\r")
    check("dispatch frame", data[STRING_FRAME - BASE:STRING_FRAME - BASE + 36]
          == b"WORLDGRID:BLACKSTART:GRID-7:WATER-3\x00")
    check("grid_deviation init", data[0x3D94:0x3D98] == struct.pack("<I", 87))
    total = sum(RESULTS)
    print(f"\n{total}/{len(RESULTS)} checks passed")
    return 0 if total == len(RESULTS) else 1


if __name__ == "__main__":
    sys.exit(main())

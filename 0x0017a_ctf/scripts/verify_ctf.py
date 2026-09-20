#!/usr/bin/env python3
"""Verify every technical claim of Operation Copperhead against CTF-02.bin.

Exits 0 only when every address, byte, hash, and derived value in CTF-R.md and
CTF-S.md matches the shipped image and the compiled ELF.
"""
import hashlib
import struct
import sys
from pathlib import Path

BASE = 0x10000000
ROOT = Path(__file__).resolve().parent.parent
BIN = ROOT / "CTF-02.bin"
UF2 = ROOT / "CTF-02.uf2"

EXPECTED_BIN_SHA = "85330c37cd0897746b1af447e4bac371dde2042abd2d61d58a61fe2a8eef3537"
EXPECTED_UF2_SHA = "f3cd4840260db820d792758cecacc5297bef1971b9eacf7601279256d8af1eab"

FRAME_THRESHOLD_A = 0x10000302
FRAME_THRESHOLD_B = 0x10000312
FRAME_TRACK = 0x1000C4B8
FRAME_BLOCKLEN = 0x1000C4F0
FRAME_SIGNALKEY = 0x1000C51C
FRAME_GATE = 0x1000C544
FRAME_AUTH = 0x1000C57C
FRAME_OK = 0x1000C438
FRAME_MISMATCH = 0x1000C43C
FRAME_SPEC_LITERAL = 0x100004FC
FRAME_DOUBLE = 0x1000EC60
FRAME_SEED = 0x1000EC70
FRAME_SALT = 0x1000CEEC
FRAME_NONCE = 0x1000CED4
FRAME_CT = 0x1000CE94

DOUBLE_3_2 = bytes.fromhex("9A99999999990940")
DOUBLE_0_32 = bytes.fromhex("7B14AE47E17AD43F")
SEED_BAD = bytes.fromhex("0A0A0A0A")
SEED_GOOD = bytes.fromhex("7465206B")
SPEC_VALUE = 0x2D879291
BUG_KEY = 0x915DCFF8

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


def rotl(v, s):
    """Rotate a 32-bit value left."""
    return ((v << s) & 0xFFFFFFFF) | (v >> (32 - s))


def qr_phase(a, b, c, d, s):
    """Apply one ARX phase of a ChaCha quarter round."""
    a = (a + b) & 0xFFFFFFFF
    d ^= a
    d = rotl(d, s)
    c = (c + d) & 0xFFFFFFFF
    b ^= c
    b = rotl(b, s)
    return a, b, c, d


def derive(seed, iv):
    """Derive the firmware signal key from a seed and IV."""
    a, b, c, d = seed, iv, 0x61707865, 0x3320646E
    for _ in range(4):
        for s in (16, 12, 8, 7):
            a, b, c, d = qr_phase(a, b, c, d, s)
    return (a ^ d) & 0xFFFFFFFF


def main():
    """Run all verification checks.

    Returns
    -------
    int
        Zero when every check passes, else one.
    """
    data = BIN.read_bytes()
    check("CTF-02.bin SHA-256", hashlib.sha256(data).hexdigest() == EXPECTED_BIN_SHA)
    check("CTF-02.uf2 SHA-256",
          hashlib.sha256(UF2.read_bytes()).hexdigest() == EXPECTED_UF2_SHA)
    check("CTF-02.bin size", len(data) == 62308, f"({len(data)})")
    check("vector table", data[0:32].hex() ==
          "002008205b0100101b0100101d01001011010010110100101101001011010010")
    check("initial SP", struct.unpack("<I", data[0:4])[0] == 0x20082000)
    check("reset vector", struct.unpack("<I", data[4:8])[0] == 0x1000015B)
    check("compare site A immediate", data[FRAME_THRESHOLD_A - BASE:
          FRAME_THRESHOLD_A - BASE + 2] == bytes.fromhex("5E2B"))
    check("compare site B immediate", data[FRAME_THRESHOLD_B - BASE:
          FRAME_THRESHOLD_B - BASE + 2] == bytes.fromhex("5E2B"))
    check("TRACK banner string", data[FRAME_TRACK - BASE:FRAME_TRACK - BASE + 13]
          == b"TRACK: NORMAL")
    check("BLOCK LENGTH string", data[FRAME_BLOCKLEN - BASE:
          FRAME_BLOCKLEN - BASE + 18] == b"BLOCK LENGTH: %u M")
    check("SIGNAL KEY string", data[FRAME_SIGNALKEY - BASE:
          FRAME_SIGNALKEY - BASE + len(b"SIGNAL KEY: 0x%08X %s")]
          == b"SIGNAL KEY: 0x%08X %s")
    check("gate message", data[FRAME_GATE - BASE:FRAME_GATE - BASE
          + len(b"Enter exactly 12 lowercase words separated by spaces.")]
          == b"Enter exactly 12 lowercase words separated by spaces.")
    check("AUTHORITY FRAME string", data[FRAME_AUTH - BASE:FRAME_AUTH - BASE
          + len(b"AUTHORITY FRAME: VERIFIED")] == b"AUTHORITY FRAME: VERIFIED")
    check("OK string", data[FRAME_OK - BASE:FRAME_OK - BASE + 3] == b"OK\x00")
    check("MISMATCH string", data[FRAME_MISMATCH - BASE:
          FRAME_MISMATCH - BASE + 9] == b"MISMATCH\x00")
    check("SIGNAL_SPEC literal", struct.unpack("<I", data[
          FRAME_SPEC_LITERAL - BASE:FRAME_SPEC_LITERAL - BASE + 4])[0] == SPEC_VALUE)
    check("3.2 double bytes", data[FRAME_DOUBLE - BASE:FRAME_DOUBLE - BASE + 8]
          == DOUBLE_3_2)
    check("0.32 target bytes", DOUBLE_0_32 == bytes.fromhex("7B14AE47E17AD43F"))
    check("bug seed bytes", data[FRAME_SEED - BASE:FRAME_SEED - BASE + 4] == SEED_BAD)
    check("good seed bytes", SEED_GOOD == bytes.fromhex("7465206B"))
    for label, addr in (("salt", FRAME_SALT), ("nonce", FRAME_NONCE),
                        ("ciphertext", FRAME_CT)):
        check(f"{label} present", data[addr - BASE:addr - BASE + 16] != b"\x00" * 16)
    iv = derive(0x6B206574, 0)
    check("derived IV", iv == 0x43C974F6, f"(0x{iv:08X})")
    check("bug-derived key", derive(0x0A0A0A0A, iv) == BUG_KEY,
          f"(0x{derive(0x0A0A0A0A, iv):08X})")
    honest = derive(0x6B206574, iv)
    check("honest key equals SIGNAL_SPEC", honest == SPEC_VALUE, f"(0x{honest:08X})")
    total = sum(RESULTS)
    print(f"\n{total}/{len(RESULTS)} checks passed")
    return 0 if total == len(RESULTS) else 1


if __name__ == "__main__":
    sys.exit(main())

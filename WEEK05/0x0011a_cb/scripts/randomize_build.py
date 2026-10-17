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
# File:    randomize_build.py
# Desc:    Builds a per-student 0x0011a_cb image with an AES-encrypted target.
# Created: 2026

"""
Per-student randomized CTF build.

Jitters the target waypoint, AES-128-ECB encrypts it under a per-build key, and
writes include/ctf_target.h so the firmware rebuilds it at boot. Every image
carries a different ciphertext and the plaintext target exists nowhere in flash.
An offline answer key or a memorized value is useless; the waypoint can only be
recovered from the artifact and confirmed on hardware.
"""

import argparse
import json
import pathlib
import random
import struct
import subprocess
import sys

BASE_LAT = 38.881940
BASE_LON = -77.450280
JITTER_DEG = 0.01
UF2_FAMILY = "0xe48bff59"
FLASH_BASE = "0x10000000"


def _parse_args() -> argparse.Namespace:
    """
    Parse command line arguments for the randomized build.

    Parameters
    ----------
    None

    Returns
    -------
    argparse.Namespace
        Parsed arguments with seed, build dir, student id, and uf2 flag.
    """
    here = pathlib.Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description="Randomized CTF build")
    parser.add_argument("--seed", type=int, default=None)
    parser.add_argument("--build-dir", default=str(here.parent / "build-ctf"))
    parser.add_argument("--student-id", default="student")
    parser.add_argument("--uf2", action="store_true")
    return parser.parse_args()


def _make_target(seed: int) -> tuple[float, float]:
    """
    Generate a jittered target waypoint around the base coordinates.

    Parameters
    ----------
    seed : int
        Deterministic seed for the jitter.

    Returns
    -------
    tuple[float, float]
        Jittered latitude and longitude, rounded to six decimals.
    """
    rng = random.Random(seed)
    lat = round(BASE_LAT + rng.uniform(-JITTER_DEG, JITTER_DEG), 6)
    lon = round(BASE_LON + rng.uniform(-JITTER_DEG, JITTER_DEG), 6)
    return lat, lon


def _make_key(seed: int) -> bytes:
    """
    Derive a deterministic 16-byte AES key for the build.

    Parameters
    ----------
    seed : int
        Build seed from which the key is derived.

    Returns
    -------
    bytes
        Sixteen byte AES-128 key.
    """
    krng = random.Random(seed ^ 0x5EED)
    return bytes(krng.randrange(256) for _ in range(16))


def _aes_ecb(plain: bytes, key: bytes) -> bytes:
    """
    Encrypt one block with AES-128-ECB.

    Parameters
    ----------
    plain : bytes
        Sixteen byte plaintext block.
    key : bytes
        Sixteen byte AES key.

    Returns
    -------
    bytes
        Sixteen byte ciphertext block.
    """
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    from cryptography.hazmat.backends import default_backend
    enc = Cipher(algorithms.AES(key), modes.ECB(), backend=default_backend()).encryptor()
    return enc.update(plain) + enc.finalize()


def _write_header(path: pathlib.Path, key: bytes, ct: bytes) -> None:
    """
    Write the AES key and ciphertext header consumed by the firmware.

    Parameters
    ----------
    path : pathlib.Path
        Destination header path.
    key : bytes
        Sixteen byte AES key.
    ct : bytes
        Sixteen byte ciphertext block.

    Returns
    -------
    None
    """
    path.write_text(
        "#ifndef CTF_TARGET_H\n#define CTF_TARGET_H\n\n#include <stdint.h>\n\n"
        "#define CTF_AES_KEY { %s }\n"
        "#define CTF_TARGET_CT { %s }\n\n"
        "#endif // CTF_TARGET_H\n"
        % (", ".join(f"0x{b:02X}" for b in key), ", ".join(f"0x{b:02X}" for b in ct)))


def _run(cmd: list[str], cwd: pathlib.Path) -> None:
    """
    Run an external command and raise on failure.

    Parameters
    ----------
    cmd : list[str]
        Command and arguments to execute.
    cwd : pathlib.Path
        Working directory for the command.

    Returns
    -------
    None
    """
    subprocess.run(cmd, cwd=str(cwd), check=True)


def main() -> int:
    """
    Build a per-student image with an AES-encrypted, randomized target.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    args = _parse_args()
    root = pathlib.Path(__file__).resolve().parent.parent
    seed = args.seed if args.seed is not None else random.randrange(2**31)
    lat, lon = _make_target(seed)
    key = _make_key(seed)
    pt = struct.pack("<d", lat) + struct.pack("<d", lon)
    ct = _aes_ecb(pt, key)

    _write_header(root / "include" / "ctf_target.h", key, ct)

    build = pathlib.Path(args.build_dir).resolve()
    _run(["cmake", "-S", str(root), "-B", str(build)], root)
    _run(["cmake", "--build", str(build)], root)

    image = build / "0x0011a_cb.bin"
    if args.uf2:
        out = build / f"0x0011a_cb_{args.student_id}.uf2"
        _run([sys.executable, str(root / "uf2conv.py"), str(image),
              "-f", UF2_FAMILY, "-b", FLASH_BASE, "-c", "-o", str(out)], root)

    key_out = {"student_id": args.student_id, "seed": seed,
               "target_lat": lat, "target_lon": lon,
               "aes_key_hex": key.hex(), "ciphertext_hex": ct.hex(),
               "image": str(image)}
    keydir = root / "scratch"
    keydir.mkdir(exist_ok=True)
    keyfile = keydir / f"answer_{args.student_id}.json"
    keyfile.write_text(json.dumps(key_out, indent=2) + "\n")

    print(f"[+] student_id : {args.student_id}")
    print(f"[+] TARGET_LAT : {lat}")
    print(f"[+] TARGET_LON : {lon}")
    print(f"[+] AES key    : {key.hex()}")
    print(f"[+] ciphertext : {ct.hex()}")
    print(f"[+] image      : {image}")
    print(f"[+] answer key : {keyfile}  (INSTRUCTOR ONLY, do not ship)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

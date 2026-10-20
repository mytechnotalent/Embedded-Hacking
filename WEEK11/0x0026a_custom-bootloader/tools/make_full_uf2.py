#!/usr/bin/env python3
"""Build a single BOOTSEL-draggable UF2 holding the whole secure-boot chain.

The 0x0026a chain is three separate images at three flash addresses:

    bootloader  0x10000000   (owns the system, verifies the next image)
    manifest    0x1001F000   (header + AES-128-CMAC tag)
    app         0x10020000   (the signed application)

The SDK emits a UF2 only for the bootloader, so a bare board would need three
separate flashes. This tool packs all three into ONE UF2 so a single BOOTSEL
drag-and-drop brings the whole device up.

How it works:

  1. Lay the three images out in a flat buffer whose offset 0 maps to
     0x10000000, filling the reserved gap between them with 0xFF (erased
     flash).
  2. Hand that flat image to picotool as a normal image at 0x10000000.
  3. picotool numbers the blocks, tags them with the rp2350-arm-s family and
     injects the RP2350-E10 absolute block that makes the boot ROM write each
     block to its absolute address.

The result is exactly the shape the boot ROM accepts over the BOOTSEL mass
storage interface, and it has been verified on hardware: the ROM accepts the
drag, the bootloader verifies the manifest, and the app runs.
"""

import argparse
import hashlib
import os
import struct
import subprocess
import sys
import tempfile

BOOTLOADER_BASE = 0x10000000
MANIFEST_BASE = 0x1001F000
APP_BASE = 0x10020000
FAMILY = "rp2350-arm-s"
PLATFORM = "rp2350"


def build_flat(bootloader, manifest, app):
    """Lay the three images out flat, offset 0 mapped to 0x10000000."""
    if APP_BASE < MANIFEST_BASE < BOOTLOADER_BASE:
        raise SystemExit("address map is not ordered")
    size = (APP_BASE - BOOTLOADER_BASE) + len(app)
    image = bytearray(b"\xFF" * size)
    image[0:len(bootloader)] = bootloader
    m_off = MANIFEST_BASE - BOOTLOADER_BASE
    image[m_off:m_off + len(manifest)] = manifest
    a_off = APP_BASE - BOOTLOADER_BASE
    image[a_off:a_off + len(app)] = app
    return bytes(image)


def convert(flat, out, picotool):
    """Run picotool to turn the flat image into an absolute-address UF2."""
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp:
        tmp.write(flat)
        flat_path = tmp.name
    try:
        cmd = [
            picotool, "uf2", "convert", flat_path, out,
            "-o", hex(BOOTLOADER_BASE),
            "--family", FAMILY,
            "--platform", PLATFORM,
            "--abs-block",
        ]
        subprocess.run(cmd, check=True)
    finally:
        os.unlink(flat_path)


def verify(out):
    """Report the block count and the families present in the UF2."""
    data = open(out, "rb").read()
    blocks = [data[i:i + 512] for i in range(0, len(data), 512)]
    families = set()
    payload = 0
    for blk in blocks:
        if len(blk) != 512 or struct.unpack_from("<I", blk, 0)[0] != 0x0A324655:
            continue
        families.add(struct.unpack_from("<I", blk, 28)[0])
        if not (struct.unpack_from("<I", blk, 8)[0] & 0x8000):
            payload += 1
    digest = hashlib.sha256(data).hexdigest()
    print("wrote %s" % out)
    print("  blocks    : %d (%d payload)" % (len(blocks), payload))
    print("  families  : %s" % ", ".join(hex(f) for f in sorted(families)))
    print("  sha256    : %s" % digest)


def main():
    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--dir", default=here, help="folder holding the images")
    ap.add_argument("--bootloader", default="0x0026a_bootloader.bin")
    ap.add_argument("--manifest", default="manifest.bin")
    ap.add_argument("--app", default="0x0026a_app.bin")
    ap.add_argument("--out", default="0x0026a_full.uf2")
    ap.add_argument("--picotool", default=os.environ.get("PICOTOOL", "picotool"),
                    help="picotool binary (env PICOTOOL)")
    args = ap.parse_args()

    bootloader = open(os.path.join(args.dir, args.bootloader), "rb").read()
    manifest = open(os.path.join(args.dir, args.manifest), "rb").read()
    app = open(os.path.join(args.dir, args.app), "rb").read()

    flat = build_flat(bootloader, manifest, app)
    out = os.path.join(args.dir, args.out)
    convert(flat, out, args.picotool)
    verify(out)


if __name__ == "__main__":
    main()

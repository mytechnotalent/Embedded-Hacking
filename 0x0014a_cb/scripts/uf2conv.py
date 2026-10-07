#!/usr/bin/env python3
r"""Convert firmware images between Intel HEX, raw BIN, and Raspberry Pi UF2.

This is the course's single canonical copy of the converter. Every week that
patches a binary refers to the copy at the repository root:

    python ../uf2conv.py build/image-h.bin --base 0x10000000 \\
        --family 0xe48bff59 --output build/hacked.uf2

It descends from the upstream Raspberry Pi ``uf2conv.py``, with the following
changes made for this course:

* Full type annotations and Google-style docstrings on every public callable.
* ``pathlib`` instead of ``os.path``, and explicit ``except`` clauses.
* Latent crash bugs in the upstream error paths fixed (see
  :func:`convert_from_uf2`).
* Module globals replaced with an explicit :class:`ConverterContext`, so the
  base address and family ID are threaded through rather than mutated behind
  the caller's back.

UF2 format, for reference
-------------------------
A UF2 file is a flat sequence of 512-byte blocks. Each block is::

    offset  size  field
    0x00    4     magicStart0 = 0x0A324655   ("UF2\\n")
    0x04    4     magicStart1 = 0x9E5D5157   (arbitrary constant)
    0x08    4     flags
    0x0C    4     targetAddr
    0x10    4     payloadSize   (max 476)
    0x14    4     blockNo
    0x18    4     numBlocks
    0x1C    4     familyID
    0x20    476   data
    0x1FC   4     magicEnd = 0x0AB16F30

Flags that matter here:

* ``0x00000001`` -- "do not flash"; this block is a no-flash marker.
* ``0x00002000`` -- "family ID present"; ``familyID`` is meaningful.

For the RP2350 the family ID is ``0xe48bff59``, and it is what tells the
boot ROM that this UF2 targets RP2350 hardware rather than, say, an RP2040 or
a Pico W.

See Also:
--------
* ``uf2families.json`` -- the family-name to family-ID table, loaded from the
  directory containing this file.
* ``flash.sh`` / ``flash.ps1`` -- program a raw ``.bin`` over SWD instead.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import struct
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from time import sleep

# ---------------------------------------------------------------------------
# UF2 format constants
# ---------------------------------------------------------------------------

#: First magic word, ``"UF2\n"`` read as a little-endian uint32.
UF2_MAGIC_START0 = 0x0A324655
#: Second magic word. The value is arbitrary but must be constant.
UF2_MAGIC_START1 = 0x9E5D5157
#: Trailing magic word closing every 512-byte block.
UF2_MAGIC_END = 0x0AB16F30

#: Size of one UF2 block in bytes.
UF2_BLOCK_SIZE = 512
#: Offset of the 32-byte block header within a block.
UF2_HEADER_SIZE = 32
#: Largest payload a single block may carry (476 = 512 - 32 header - 4 magic).
UF2_MAX_PAYLOAD = UF2_BLOCK_SIZE - UF2_HEADER_SIZE - 4

#: Flag bit marking a block the bootloader must NOT program.
UF2_FLAG_NOFLASH = 0x00000001
#: Flag bit announcing that ``familyID`` carries a meaningful value.
UF2_FLAG_FAMILY_ID = 0x00002000

#: Name of the marker file the boot ROM exposes on its flash-drive volume.
INFO_FILE = "INFO_UF2.TXT"

#: Struct format for the 32-byte UF2 block header.
_HEADER_STRUCT = struct.Struct("<IIIIIIII")

#: The default family ID assumed when the caller does not pass ``--family``.
DEFAULT_FAMILY_ID = 0x0
#: The default base address assumed when the caller does not pass ``--base``.
#: Matches the upstream default; RP2350 firmware always overrides this.
DEFAULT_BASE_ADDRESS = 0x2000


class Uf2Error(Exception):
    """Raised when an input file is malformed or internally inconsistent."""


@dataclass
class ConverterContext:
    """Mutable conversion state shared across a single CLI invocation.

    Attributes:
        appstartaddr: Base address that payload offsets are relative to. Set
            from ``--base``; ``0`` means "derive it from the input".
        familyid: Family ID written into every UF2 block. ``0`` means "no
            family flag", which is the correct behaviour for a plain BIN->UF2
            conversion of RP2040-era images, and the wrong behaviour for
            RP2350, which is why the course always passes ``--family``.
    """

    appstartaddr: int | None = DEFAULT_BASE_ADDRESS
    familyid: int = DEFAULT_FAMILY_ID


# ---------------------------------------------------------------------------
# Format detection
# ---------------------------------------------------------------------------


def is_uf2(buf: bytes) -> bool:
    """Report whether a buffer begins with the UF2 start magic.

    Args:
        buf: Raw file contents.

    Returns:
        ``True`` if the first two uint32 words are the UF2 start magic.
    """
    if len(buf) < 8:
        return False
    word0, word1 = _HEADER_STRUCT.unpack_from(buf, 0)[:2]
    return word0 == UF2_MAGIC_START0 and word1 == UF2_MAGIC_START1


def is_hex(buf: bytes) -> bool:
    """Report whether a buffer looks like an Intel HEX file.

    A file qualifies when it decodes as UTF-8, begins with a record marker
    ``:``, and contains only characters legal in Intel HEX records.

    Args:
        buf: Raw file contents.

    Returns:
        ``True`` if the buffer should be treated as Intel HEX.
    """
    try:
        text = buf[:30].decode("utf-8")
    except UnicodeDecodeError:
        return False
    if not text.startswith(":"):
        return False
    return re.match(rb"^[:0-9a-fA-F\r\n]+$", buf) is not None


# ---------------------------------------------------------------------------
# UF2 -> BIN
# ---------------------------------------------------------------------------


def convert_from_uf2(buf: bytes, ctx: ConverterContext) -> bytes:
    """Decode a UF2 file back into a flat binary image.

    Blocks are visited in file order. Gaps between consecutive block addresses
    are filled with zero words, and any trailing gap is dropped so that the
    result ends at the last byte actually programmed.

    A UF2 file may legitimately interleave blocks from several families (this
    is how a multicore image ships both cores in one download). When more than
    one family is present and the caller did not constrain the family with
    ``--family``, the conversion is ambiguous, so the payload is emptied and
    ``appstartaddr`` reset to ``0``.

    Args:
        buf: Complete UF2 file contents.
        ctx: Conversion state; updated in place with the discovered base
            address.

    Returns:
        The decoded payload.

    Raises:
        Uf2Error: If a block declares an oversized payload, the blocks are out
            of order, or the implied padding exceeds 10 MiB.

    Note:
        The upstream Raspberry Pi version of this function crashes on its own
        error paths: it builds the message with ``"..." + ptr`` where ``ptr``
        is an ``int``, which raises :class:`TypeError` and masks the real
        problem. The messages here use f-strings so the diagnostic survives.
    """
    numblocks = len(buf) // UF2_BLOCK_SIZE
    curraddr: int | None = None
    currfamilyid: int | None = None
    families_found: dict[int, int] = {}
    prev_flag: int | None = None
    all_flags_same = True
    outp: list[bytes] = []

    for blockno in range(numblocks):
        ptr = blockno * UF2_BLOCK_SIZE
        block = buf[ptr : ptr + UF2_BLOCK_SIZE]
        (
            magic0,
            magic1,
            flags,
            target_addr,
            datalen,
            _blockno,
            _numblocks,
            block_family,
        ) = _HEADER_STRUCT.unpack_from(block, 0)

        if magic0 != UF2_MAGIC_START0 or magic1 != UF2_MAGIC_START1:
            print(f"Skipping block at {ptr:#x}; bad magic")
            continue

        if flags & UF2_FLAG_NOFLASH:
            continue

        if datalen > UF2_MAX_PAYLOAD:
            msg = f"Invalid UF2 data size at {ptr:#x}: {datalen}"
            raise Uf2Error(msg)

        if flags & UF2_FLAG_FAMILY_ID and currfamilyid is None:
            currfamilyid = block_family

        # A new contiguous run starts when the address jumps or the family
        # changes. appstartaddr tracks the start of the current run, which is
        # what makes the reported start address the image's true base rather
        # than the address of its last block.
        if curraddr is None or (
            flags & UF2_FLAG_FAMILY_ID and block_family != currfamilyid
        ):
            currfamilyid = block_family
            curraddr = target_addr
            if ctx.familyid == DEFAULT_FAMILY_ID or ctx.familyid == block_family:
                ctx.appstartaddr = target_addr

        padding = target_addr - curraddr
        if padding < 0:
            msg = f"Block out of order at {ptr:#x}: {target_addr:#x} < {curraddr:#x}"
            raise Uf2Error(msg)
        if padding > 10 * 1024 * 1024:
            msg = f"More than 10M of padding needed at {ptr:#x}"
            raise Uf2Error(msg)
        if padding % 4 != 0:
            msg = f"Non-word padding size at {ptr:#x}: {padding}"
            raise Uf2Error(msg)

        while padding > 0:
            padding -= 4
            outp.append(b"\x00\x00\x00\x00")

        if ctx.familyid == DEFAULT_FAMILY_ID or (
            flags & UF2_FLAG_FAMILY_ID and ctx.familyid == block_family
        ):
            outp.append(
                block[UF2_HEADER_SIZE : UF2_HEADER_SIZE + datalen],
            )

        curraddr = target_addr + datalen

        if flags & UF2_FLAG_FAMILY_ID:
            existing = families_found.get(block_family)
            if existing is None or existing > target_addr:
                families_found[block_family] = target_addr

        if prev_flag is None:
            prev_flag = flags
        elif prev_flag != flags:
            all_flags_same = False

        if blockno == numblocks - 1:
            _print_uf2_header_info(families_found, all_flags_same, flags)
            if len(families_found) > 1 and ctx.familyid == DEFAULT_FAMILY_ID:
                outp = []
                ctx.appstartaddr = 0x0

    return b"".join(outp)


def _print_uf2_header_info(
    families_found: dict[int, int],
    all_flags_same: bool,
    flags: int,
) -> None:
    """Print the family summary block that follows a UF2->BIN conversion.

    Args:
        families_found: Mapping of family ID to the lowest target address seen
            for that family.
        all_flags_same: Whether every block carried an identical flag word.
        flags: The flag word from the final block, used for display.
    """
    print("--- UF2 File Header Info ---")
    families = load_families()
    name_by_id = {value: key for key, value in families.items()}
    for family_hex, address in families_found.items():
        short_name = name_by_id.get(family_hex, "")
        print(f"Family ID is {short_name}, hex value is {family_hex:#010x}")
        print(f"Target Address is {address:#010x}")
    if all_flags_same:
        print(f"All block flag values consistent, {flags:#06x}")
    else:
        print("Flags were not all the same")
    print("----------------------------")


# ---------------------------------------------------------------------------
# BIN -> UF2
# ---------------------------------------------------------------------------


def convert_to_uf2(file_content: bytes, ctx: ConverterContext) -> bytes:
    """Wrap a raw binary in UF2 blocks.

    The binary is split into 256-byte payloads; the final block is zero-padded.
    Each block's ``targetAddr`` is the byte offset within the image plus
    :attr:`ConverterContext.appstartaddr`, so for RP2350 firmware built at
    ``0x10000000`` the caller passes ``--base 0x10000000``.

    Args:
        file_content: The raw image.
        ctx: Conversion state supplying the base address and family ID.

    Returns:
        The UF2 file contents, always a whole number of 512-byte blocks.
    """
    datapadding = bytes(UF2_BLOCK_SIZE - 256 - UF2_HEADER_SIZE - 4)
    numblocks = (len(file_content) + 255) // 256
    flags = UF2_FLAG_FAMILY_ID if ctx.familyid else 0x0
    base = ctx.appstartaddr or 0

    blocks: list[bytes] = []
    for blockno in range(numblocks):
        ptr = 256 * blockno
        chunk = file_content[ptr : ptr + 256]
        header = _HEADER_STRUCT.pack(
            UF2_MAGIC_START0,
            UF2_MAGIC_START1,
            flags,
            ptr + base,
            256,
            blockno,
            numblocks,
            ctx.familyid,
        )
        payload = chunk + bytes(256 - len(chunk))
        block = header + payload + datapadding + struct.pack("<I", UF2_MAGIC_END)
        blocks.append(block)
    return b"".join(blocks)


def convert_to_carray(file_content: bytes) -> bytes:
    """Render a binary as a C array initialiser for embedding in firmware.

    Args:
        file_content: The raw image.

    Returns:
        UTF-8 bytes of a C translation unit fragment.
    """
    parts = [
        f"const unsigned long bindata_len = {len(file_content)};\n",
        "const unsigned char bindata[] __attribute__((aligned(16))) = {",
    ]
    for index, value in enumerate(file_content):
        if index % 16 == 0:
            parts.append("\n")
        parts.append(f"{value:#04x}, ")
    parts.append("\n};\n")
    return "".join(parts).encode()


# ---------------------------------------------------------------------------
# Intel HEX -> UF2
# ---------------------------------------------------------------------------


@dataclass
class Block:
    """A single 256-byte-aligned region of an image under construction.

    Attributes:
        addr: Base address of the block, always 256-byte aligned.
        data: The block's bytes, initialised to ``default_data``.
    """

    addr: int
    data: bytearray

    def __init__(self, addr: int, default_data: int = 0xFF) -> None:
        """Initialise an erased 256-byte block.

        Args:
            addr: 256-byte-aligned base address of the block.
            default_data: Fill byte; ``0xFF`` matches erased flash.
        """
        self.addr = addr
        self.data = bytearray([default_data] * 256)

    def encode(self, blockno: int, numblocks: int, ctx: ConverterContext) -> bytes:
        """Serialise this block into a 512-byte UF2 block.

        Args:
            blockno: Index of this block within the file.
            numblocks: Total number of blocks in the file.
            ctx: Conversion state supplying the family ID.

        Returns:
            Exactly 512 bytes.
        """
        flags = UF2_FLAG_FAMILY_ID if ctx.familyid else 0x0
        out = _HEADER_STRUCT.pack(
            UF2_MAGIC_START0,
            UF2_MAGIC_START1,
            flags,
            self.addr,
            256,
            blockno,
            numblocks,
            ctx.familyid,
        )
        out += self.data[0:256]
        out += bytes(UF2_BLOCK_SIZE - 4 - len(out))
        out += struct.pack("<I", UF2_MAGIC_END)
        return out


def convert_from_hex_to_uf2(text: str, ctx: ConverterContext) -> bytes:
    """Assemble Intel HEX records into a UF2 file.

    Only record types 0x00 (data), 0x01 (EOF), 0x02 (extended segment address)
    and 0x04 (extended linear address) are handled; any other type is ignored,
    matching the upstream behaviour.

    Args:
        text: Decoded Intel HEX file contents.
        ctx: Conversion state; ``appstartaddr`` is set to the first data
            address seen.

    Returns:
        The UF2 file contents.
    """
    ctx.appstartaddr = None
    upper = 0
    currblock: Block | None = None
    blocks: list[Block] = []

    for line in text.split("\n"):
        if not line.startswith(":"):
            continue
        record = [int(line[i : i + 2], 16) for i in range(1, len(line) - 1, 2)]
        rec_type = record[3]
        if rec_type == 4:
            upper = ((record[4] << 8) | record[5]) << 16
        elif rec_type == 2:
            upper = ((record[4] << 8) | record[5]) << 4
        elif rec_type == 1:
            break
        elif rec_type == 0:
            addr = upper + ((record[1] << 8) | record[2])
            if ctx.appstartaddr is None:
                ctx.appstartaddr = addr
            index = 4
            while index < len(record) - 1:
                if currblock is None or currblock.addr & ~0xFF != addr & ~0xFF:
                    currblock = Block(addr & ~0xFF)
                    blocks.append(currblock)
                currblock.data[addr & 0xFF] = record[index]
                addr += 1
                index += 1

    numblocks = len(blocks)
    return b"".join(blocks[i].encode(i, numblocks, ctx) for i in range(numblocks))


# ---------------------------------------------------------------------------
# Drive discovery
# ---------------------------------------------------------------------------


def _iter_candidate_dirs() -> list[Path]:
    """Return the directories that may contain mounted removable volumes."""
    if sys.platform == "win32":
        return []

    searchpaths = [Path("/mnt"), Path("/media")]
    if sys.platform == "darwin":
        searchpaths = [Path("/Volumes")]
    elif sys.platform == "linux":
        user = environ_user()
        if user:
            searchpaths += [Path("/media") / user, Path("/run/media") / user]
        sudo_user = os.environ.get("SUDO_USER")
        if sudo_user:
            searchpaths += [
                Path("/media") / sudo_user,
                Path("/run/media") / sudo_user,
            ]
    return searchpaths


def environ_user() -> str | None:
    """Return the current user name from the environment.

    Tried under several variable names so the search works on macOS, Linux and
    Windows shells alike.

    Returns:
        The user name, or ``None`` if it cannot be determined.
    """
    for var in ("USER", "USERNAME", "LOGNAME"):
        value = os.environ.get(var)
        if value:
            return value
    return None


def get_drives() -> list[Path]:
    """Find mounted Pico boot drives.

    A drive qualifies when it contains the ``INFO_UF2.TXT`` marker the boot ROM
    writes when it presents the flash as a USB mass-storage device.

    Returns:
        Every qualifying mount point.
    """
    drives: list[Path] = []

    if sys.platform == "win32":
        command = (
            "(Get-Volume | Where-Object { $_.FileSystemLabel -match 'RP2350|RPI-RP2' })"
            ".DriveLetter"
        )
        try:
            raw = subprocess.check_output(
                ["powershell", "-Command", command],
                text=True,
            )
        except (subprocess.CalledProcessError, FileNotFoundError, OSError):
            return []
        for letter in raw.split():
            if len(letter) == 1 and letter.isalpha():
                drives.append(Path(f"{letter.upper()}:\\"))
        return drives

    for rootpath in _iter_candidate_dirs():
        if not rootpath.is_dir():
            continue
        try:
            entries = list(rootpath.iterdir())
        except OSError:
            continue
        for entry in entries:
            if not entry.is_dir():
                continue
            if (entry / INFO_FILE).is_file():
                drives.append(entry)
    return drives


def board_id(path: Path) -> str:
    """Read the ``Board-ID`` field from a boot drive's info file.

    Args:
        path: Mount point of the boot drive.

    Returns:
        The board ID string, for example ``RP2350``.

    Raises:
        Uf2Error: If the info file is missing or has no ``Board-ID`` field.
    """
    info = path / INFO_FILE
    if not info.is_file():
        msg = f"Not a Pico boot drive: {path} has no {INFO_FILE}"
        raise Uf2Error(msg)
    match = re.search(r"Board-ID: ([^\r\n]*)", info.read_text())
    if match is None:
        msg = f"No Board-ID field in {info}"
        raise Uf2Error(msg)
    return match.group(1)


def list_drives() -> None:
    """Print every connected Pico boot drive and its board ID."""
    for drive in get_drives():
        try:
            print(drive, board_id(drive))
        except Uf2Error as exc:
            print(f"{drive} <unreadable: {exc}>")


def write_file(name: Path | str, buf: bytes) -> None:
    """Write a buffer to disk and report the size written.

    Args:
        name: Destination path.
        buf: Bytes to write.
    """
    path = Path(name)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(buf)
    print(f"Wrote {len(buf)} bytes to {path}")


def load_families() -> dict[str, int]:
    """Load the family-name to family-ID table.

    ``uf2families.json`` is read from the directory containing this script, so
    the converter works regardless of the caller's working directory.

    Returns:
        Mapping of upper-case short name to family ID.

    Raises:
        Uf2Error: If the JSON file is missing or malformed.
    """
    pathname = Path(__file__).resolve().parent / "uf2families.json"
    try:
        raw_families = json.loads(pathname.read_text())
    except FileNotFoundError as exc:
        msg = f"Required family table not found: {pathname}"
        raise Uf2Error(msg) from exc
    except json.JSONDecodeError as exc:
        msg = f"Malformed family table {pathname}: {exc}"
        raise Uf2Error(msg) from exc

    return {fam["short_name"]: int(fam["id"], 0) for fam in raw_families}


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def _build_parser() -> argparse.ArgumentParser:
    """Construct the command-line argument parser.

    Returns:
        A configured :class:`argparse.ArgumentParser`.
    """
    parser = argparse.ArgumentParser(
        prog="uf2conv.py",
        description="Convert firmware between Intel HEX, raw BIN and UF2.",
        epilog=(
            "Course example (RP2350 raw image built for XIP flash):\n"
            "  python ../uf2conv.py build/image-h.bin "
            "--base 0x10000000 \\\n"
            "      --family 0xe48bff59 --output build/hacked.uf2\n"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "input",
        metavar="INPUT",
        nargs="?",
        help="input file (HEX, BIN or UF2); omit when using --list",
    )
    parser.add_argument(
        "-b",
        "--base",
        default=hex(DEFAULT_BASE_ADDRESS),
        help=(
            "base address of the application for BIN input "
            f"(default: {hex(DEFAULT_BASE_ADDRESS)}; use 0x10000000 for RP2350 XIP)"
        ),
    )
    parser.add_argument(
        "-f",
        "--family",
        default=hex(DEFAULT_FAMILY_ID),
        help=(
            "family ID as a number or a name from uf2families.json "
            f"(default: {hex(DEFAULT_FAMILY_ID)}; RP2350 is 0xe48bff59)"
        ),
    )
    parser.add_argument(
        "-o",
        "--output",
        metavar="FILE",
        help='write output to FILE; defaults to "flash.uf2" or "flash.bin"',
    )
    parser.add_argument(
        "-d",
        "--device",
        dest="device_path",
        help="select a specific device path to flash (reserved; unused)",
    )
    parser.add_argument(
        "-l",
        "--list",
        action="store_true",
        help="list connected Pico boot drives and exit",
    )
    parser.add_argument(
        "-c",
        "--convert",
        action="store_true",
        help="convert only; do not deploy to a mounted boot drive",
    )
    parser.add_argument(
        "-D",
        "--deploy",
        action="store_true",
        help="deploy the input file unchanged; do not convert",
    )
    parser.add_argument(
        "-w",
        "--wait",
        action="store_true",
        help="poll for a boot drive to appear instead of failing immediately",
    )
    parser.add_argument(
        "-C",
        "--carray",
        action="store_true",
        help="emit a C array initialiser instead of a UF2 file",
    )
    parser.add_argument(
        "-i",
        "--info",
        action="store_true",
        help="print UF2 header information and exit without converting",
    )
    return parser


def _resolve_family(spec: str) -> int:
    """Resolve a ``--family`` value to a numeric family ID.

    Args:
        spec: Either a family short name (case-insensitive) or an integer
            literal such as ``0xe48bff59``.

    Returns:
        The numeric family ID.

    Raises:
        Uf2Error: If the value is neither a known name nor a valid integer.
    """
    families = load_families()
    if spec.upper() in families:
        return families[spec.upper()]
    try:
        return int(spec, 0)
    except ValueError as exc:
        msg = "Family ID needs to be a number or one of: " + ", ".join(families)
        raise Uf2Error(msg) from exc


def _wait_for_drive() -> Path | None:
    """Poll for a boot drive to be mounted.

    Returns:
        The first drive found, or ``None`` if the wait was interrupted.
    """
    print("Waiting for drive to deploy...")
    while True:
        drives = get_drives()
        if drives:
            return drives[0]
        sleep(0.1)


def main(argv: list[str] | None = None) -> int:
    """Run the command-line converter.

    Args:
        argv: Argument list, defaulting to :data:`sys.argv` ``[1:]``.

    Returns:
        ``0`` on success, ``1`` on a user or input error.
    """
    parser = _build_parser()
    args = parser.parse_args(argv)

    ctx = ConverterContext()
    try:
        ctx.familyid = _resolve_family(args.family)
        ctx.appstartaddr = int(args.base, 0)
    except (Uf2Error, ValueError) as exc:
        print(exc, file=sys.stderr)
        return 1

    if args.list:
        list_drives()
        return 0

    if not args.input:
        print("Need an input file (or use --list)", file=sys.stderr)
        return 1

    source = Path(args.input)
    if not source.is_file():
        print(f"error: file not found: {source}", file=sys.stderr)
        return 1

    inpbuf = source.read_bytes()
    from_uf2 = is_uf2(inpbuf)
    ext = "uf2"

    try:
        if args.deploy:
            outbuf = inpbuf
        elif from_uf2 and not args.info:
            outbuf = convert_from_uf2(inpbuf, ctx)
            ext = "bin"
        elif from_uf2 and args.info:
            outbuf = b""
            convert_from_uf2(inpbuf, ctx)
        elif is_hex(inpbuf):
            outbuf = convert_from_hex_to_uf2(inpbuf.decode("utf-8"), ctx)
        elif args.carray:
            outbuf = convert_to_carray(inpbuf)
            ext = "h"
        else:
            outbuf = convert_to_uf2(inpbuf, ctx)
    except Uf2Error as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    if not args.deploy and not args.info:
        print(
            f"Converted to {ext}, output size: {len(outbuf)}, "
            f"start address: {ctx.appstartaddr or 0:#x}",
        )

    if (args.convert or ext != "uf2") and args.output is None:
        args.output = f"flash.{ext}"
    if args.output:
        write_file(args.output, outbuf)

    if ext == "uf2" and not args.convert and not args.info:
        drives = get_drives()
        if not drives:
            if args.wait:
                found = _wait_for_drive()
                drives = [found] if found else []
            elif not args.output:
                print("error: no drive to deploy", file=sys.stderr)
                return 1
        for drive in drives:
            print(f"Flashing {drive} ({board_id(drive)})")
            write_file(drive / "NEW.UF2", outbuf)

    return 0


if __name__ == "__main__":
    sys.exit(main())

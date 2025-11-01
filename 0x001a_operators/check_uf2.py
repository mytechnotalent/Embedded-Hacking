"""
Verify patched instruction and constant inside a UF2 file.

Checks:
- 0x10000410: should be 26 7a b4 ee (vsub.f32 s14, s14, s11)
- 0x10000414: should be a6 7a b4 ee (vsub.f32 s15, s15, s11)
- 0x1000042C: should be 00 00 00 41 (float 8.0)

Usage:
    python verify_uf2.py build/hacked.uf2
"""

import sys
import struct

UF2_MAGIC_START0 = 0x0A324655  # "UF2\n"
UF2_MAGIC_START1 = 0x9E5D5157
UF2_MAGIC_END    = 0x0AB16F30

def load_uf2(path):
    blocks = []
    with open(path, "rb") as f:
        data = f.read()
    i = 0
    while i + 512 <= len(data):
        blk = data[i:i+512]
        (m0, m1, flags, tgt_addr, payload_size, block_no,
         num_blocks, file_size) = struct.unpack("<IIIIIIII", blk[:32])
        if m0 != UF2_MAGIC_START0 or m1 != UF2_MAGIC_START1:
            raise ValueError("Not a UF2 file or bad magic at block %d" % (i // 512))
        if payload_size != 256:
            raise ValueError("Unexpected payload size %d" % payload_size)
        if struct.unpack("<I", blk[512-4:512])[0] != UF2_MAGIC_END:
            raise ValueError("Bad end magic at block %d" % (i // 512))
        payload = blk[32:32+payload_size]
        blocks.append((tgt_addr, payload))
        i += 512
    return blocks

def read_at(blocks, addr, length):
    """Read 'length' bytes starting at virtual 'addr' from UF2 blocks."""
    out = bytearray()
    cur = addr
    while len(out) < length:
        # Find block that contains current address
        hit = None
        for base, pay in blocks:
            if base <= cur < base + len(pay):
                hit = (base, pay)
                break
        if not hit:
            # If missing, fill with 0xFF (flash default) and continue
            out.append(0xFF)
            cur += 1
            continue
        base, pay = hit
        out.append(pay[cur - base])
        cur += 1
    return bytes(out)

def main():
    if len(sys.argv) != 2:
        print("Usage: python verify_uf2.py <path/to.uf2>")
        sys.exit(1)
    uf2_path = sys.argv[1]
    blocks = load_uf2(uf2_path)

    addrs = [0x10000410, 0x10000414, 0x1000042C]
    labels = ["@10000410", "@10000414", "@1000042C"]

    for label, a in zip(labels, addrs):
        bytes_ = read_at(blocks, a, 4)
        print(f"{label}: {bytes_.hex()} ", end="")
        if a == 0x1000042C:
            try:
                print(f"(float {struct.unpack('<f', bytes_)[0]:.6f})")
            except Exception:
                print("(not a valid float)")
        else:
            print()

if __name__ == "__main__":
    main()

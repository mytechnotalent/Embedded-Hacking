# 0x0026a Custom AES-128-CMAC Secure Bootloader

This project builds a custom secure-boot chain for the Pico 2 (RP2350) that sits entirely above the ROM bootloader. A first-stage bootloader in XIP flash authenticates a signed application with AES-128-CMAC (RFC 4493) before jumping to it, and the signed application decrypts and prints a secret banner using the SDK Mbed TLS AES API.

## What You'll Learn

- Why a bootloader-only CRC/manifest is insufficient and what a MAC adds
- How AES-128-CMAC works (K1/K2 subkeys) and the RFC 4493 test vectors
- Custom linker-script editing (`pico_flash_region.ld`, `memmap_app.ld`) to relocate the app
- Flashing three images (bootloader, manifest, app) with OpenOCD
- Reverse engineering both images with GDB and Ghidra (find the key, IV, ciphertext, banner)

See the full tutorial: [WEEK11a.md](WEEK11a.md) / [WEEK11a.pdf](WEEK11a.pdf)

## Flash Map

| Region     | Address     | Content                        |
| ---------- | ----------- | ------------------------------ |
| Bootloader | `0x10000000` | 0x0026a bootloader (27772 B)  |
| Manifest   | `0x1001F000` | header + AES-128-CMAC tag     |
| App        | `0x10020000` | 0x0026a signed app (21656 B)  |

The CMSIS-RM9 flash-safe range starts at `0x10000000` on the RP2350, so adding 2 MB relocates the app to `0x10020000`.

## Artifacts

| File                         | Description                                       |
| ---------------------------- | ------------------------------------------------- |
| `0x0026a_bootloader.bin`     | Bootloader image (flash at `0x10000000`)          |
| `0x0026a_bootloader.uf2`     | Bootloader-only UF2 for the BOOTSEL drag-and-drop |
| `0x0026a_app.bin`            | Signed app image (flash at `0x10020000`)          |
| `manifest.bin`               | Manifest (flash at `0x1001F000`)                  |
| `0x0026a_full.uf2`           | **All three images** in one BOOTSEL drag          |
| `WEEK11a.md` / `WEEK11a.pdf` | Full lab notebook                                 |

## Build

```sh
cmake -B build -DPICO_SDK_PATH=~/.pico-sdk/sdk/2.2.0
cmake --build build
python3 tools/sign_firmware.py build/0x0026a_app.bin --out manifest.bin --key "IronNetBootKey!!" --app-base 0x10020000
PICOTOOL=~/.pico-sdk/picotool/2.3.1/picotool/picotool python3 tools/make_full_uf2.py
```

This produces `0x0026a_bootloader.bin/.uf2` and `0x0026a_app.bin`; the signer
writes `manifest.bin`; `make_full_uf2.py` packs all three into
`0x0026a_full.uf2`.

## Flash

Easiest - copy `0x0026a_full.uf2` to the `RP2350` BOOTSEL drive; one drag writes
the whole chain (bootloader + manifest + app).

Or with OpenOCD:

```sh
openocd -f interface/picoprobe.cfg -f target/rp2350.cfg -c \
  "program 0x0026a_bootloader.bin 0x10000000 verify; \
   program manifest.bin 0x1001F000 verify; \
   program 0x0026a_app.bin 0x10020000 verify; reset run; shutdown"
```

## Verify at Runtime

Halt the core after boot and read the program counter. A valid signed image leaves PC inside the app (`0x100201xx`); a tampered or unsigned image leaves PC in the bootloader `boot_halt` idle loop near `0x10000eb2`.

## Disclaimers

This is an educational project for the Embedded-Hacking course. The key `IronNetBootKey!!` ships in the repo and is intentionally embedded in both images so it can be recovered with GDB/Ghidra. Do not use this scheme to protect real products.
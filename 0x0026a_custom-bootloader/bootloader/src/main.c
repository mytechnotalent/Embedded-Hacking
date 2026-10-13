/**
 * @file main.c
 * @brief Secure bootloader entry point (verify AES-128-CMAC, then boot).
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 *
 * -----------------------------------------------------------------------------
 *
 * This file is deliberately thin: it only sequences the secure boot steps.
 * The console setup, halt reporting, and application handoff live in boot.c;
 * the CMAC verification (and its key) live in verify.c; the manifest lives in
 * manifest.c; and the raw reset-vector jump lives in image.c.
 *
 * Flash map (2 MiB, RP2350 Pico 2):
 *   0x10000000  bootloader image (this program)
 *   0x1001F000  one-sector firmware manifest (magic, size, CMAC tag)
 *   0x10020000  signed application image
 */

#include "manifest.h"
#include "boot.h"
#include "verify.h"

int main(void) {
    fw_manifest_t manifest;
    boot_init();
    manifest_read(&manifest);
    if (!manifest_is_valid(&manifest)) boot_halt("manifest invalid");
    if (!verify_manifest(&manifest)) boot_halt("firmware signature invalid");
    boot_launch(&manifest);
}
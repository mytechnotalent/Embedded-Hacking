/**
 * @file verify.c
 * @brief Firmware authenticity verification for the secure bootloader.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include <stdint.h>
#include "flash_layout.h"
#include "verify.h"
#include "aes_cmac.h"

/** @brief AES-128 key that authenticates the application image. */
static const uint8_t fw_key[FW_TAG_SIZE] = {
    0x49, 0x72, 0x6F, 0x6E, 0x4E, 0x65, 0x74, 0x42,
    0x6F, 0x6F, 0x74, 0x4B, 0x65, 0x79, 0x21, 0x21
};

bool verify_manifest(const fw_manifest_t *manifest) {
    const uint8_t *base = (const uint8_t *)manifest->app_base;
    uint8_t computed[FW_TAG_SIZE];
    int rc = aes128_cmac(fw_key, base, manifest->app_size, computed);
    if (rc != 0) return false;
    return cmac_tags_equal(computed, manifest->tag);
}
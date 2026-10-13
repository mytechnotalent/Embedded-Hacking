/**
 * @file manifest.c
 * @brief Reads and validates the firmware manifest from XIP flash.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "flash_layout.h"
#include "manifest.h"

void manifest_read(fw_manifest_t *out) {
    const fw_manifest_t *src = (const fw_manifest_t *)FW_MANIFEST_BASE;
    *out = *src;
}

bool manifest_is_valid(const fw_manifest_t *manifest) {
    if (manifest->magic != FW_MANIFEST_MAGIC) return false;
    if (manifest->version != FW_MANIFEST_VERSION) return false;
    return true;
}

/**
 * @file manifest.h
 * @brief Firmware manifest stored in the flash sector before the application.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef MANIFEST_H
#define MANIFEST_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Little-endian magic at the head of a valid manifest ("WFLB"). */
#define FW_MANIFEST_MAGIC 0x424C4657U

/** @brief Current manifest structure revision. */
#define FW_MANIFEST_VERSION 1U

/** @brief Length in bytes of an AES-128-CMAC authentication tag. */
#define FW_TAG_SIZE 16U

#pragma pack(push, 1)
/** @brief Fixed-layout record describing the signed application image. */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t app_base;
    uint32_t app_size;
    uint8_t  tag[FW_TAG_SIZE];
} fw_manifest_t;
#pragma pack(pop)

/**
 * @brief Copy the manifest out of its fixed flash address.
 * @param out Pointer to the manifest structure to populate.
 * @return void No return value.
 */
void manifest_read(fw_manifest_t *out);

/**
 * @brief Report whether a manifest carries the expected magic and version.
 * @param manifest Pointer to the manifest to inspect.
 * @return bool True when the magic and version both match.
 */
bool manifest_is_valid(const fw_manifest_t *manifest);

#endif

/**
 * @file verify.h
 * @brief Firmware authenticity verification for the secure bootloader.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef VERIFY_H
#define VERIFY_H

#include <stdbool.h>
#include "manifest.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Recompute the CMAC over the application and compare it to the tag.
 * @param manifest Pointer to the validated manifest.
 * @return bool True when the tag matches, false otherwise.
 */
bool verify_manifest(const fw_manifest_t *manifest);

#ifdef __cplusplus
}
#endif

#endif // VERIFY_H
/**
 * @file boot.h
 * @brief Secure boot console and application handoff helpers.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef BOOT_H
#define BOOT_H

#include "manifest.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bring up the debug console and print the boot banner.
 * @param void No parameters.
 * @return void No return value.
 */
void boot_init(void);

/**
 * @brief Print a boot failure reason and idle forever.
 * @param reason Null-terminated message describing the failure.
 * @return void Does not return.
 */
void boot_halt(const char *reason);

/**
 * @brief Announce the verified image and jump to its reset vector.
 * @param manifest Pointer to the validated manifest.
 * @return void Does not return.
 */
void boot_launch(const fw_manifest_t *manifest);

#ifdef __cplusplus
}
#endif

#endif // BOOT_H
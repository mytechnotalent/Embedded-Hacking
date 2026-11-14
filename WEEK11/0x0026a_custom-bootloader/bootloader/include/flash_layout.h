/**
 * @file flash_layout.h
 * @brief Flash partition map shared by the bootloader and the host signer.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef FLASH_LAYOUT_H
#define FLASH_LAYOUT_H

#include <stdint.h>

/** @brief Absolute base address of the bootloader image in XIP flash. */
#define BOOTLOADER_BASE 0x10000000U

/** @brief Absolute address of the one-sector firmware manifest. */
#define FW_MANIFEST_BASE 0x1001F000U

/** @brief Size in bytes of the flash sector holding the manifest. */
#define FW_MANIFEST_SECTOR_SIZE 4096U

/** @brief Absolute base address where the signed application runs. */
#define APP_BASE 0x10020000U

/** @brief Total XIP flash size in bytes on the RP2350 Pico 2 board. */
#define FLASH_TOTAL_SIZE 0x200000U

#endif

/**
 * @file flash.h
 * @brief Header for on-chip flash read/write driver (XIP memory-mapped)
 * @author Kevin Thomas
 * @date 2025
 *
 * MIT License
 *
 * Copyright (c) 2025 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Total on-chip flash size in bytes (4 MB) */
#define FLASH_DRIVER_SIZE_BYTES   (4 * 1024 * 1024)
/** @brief Flash erase sector size in bytes */
#define FLASH_DRIVER_SECTOR_SIZE  4096
/** @brief Flash program page size in bytes */
#define FLASH_DRIVER_PAGE_SIZE    256

/**
 * @brief Erase one 4096-byte sector and write data to on-chip flash
 *
 * The target address must be aligned to a 4096-byte sector boundary.
 * The function disables interrupts, erases the containing sector,
 * programs up to @p len bytes from @p data, and re-enables interrupts. The
 * write length must be a multiple of FLASH_DRIVER_PAGE_SIZE (256 bytes);
 * pad with 0xFF if necessary.
 *
 * @param flash_offset Byte offset from the start of flash (must be sector-aligned)
 * @param data         Pointer to the data buffer to write
 * @param len          Number of bytes to write (multiple of FLASH_DRIVER_PAGE_SIZE)
 */
void flash_driver_write(uint32_t flash_offset, const uint8_t *data, uint32_t len);

/**
 * @brief Read bytes from on-chip flash via the XIP memory map
 *
 * Flash is memory-mapped starting at XIP_BASE (0x10000000). This function
 * copies @p len bytes beginning at @p flash_offset into @p out using the
 * XIP read path, which is always available without erasing.
 *
 * @param flash_offset Byte offset from the start of flash
 * @param out          Pointer to the destination buffer (must be @p len bytes)
 * @param len          Number of bytes to read
 */
void flash_driver_read(uint32_t flash_offset, uint8_t *out, uint32_t len);

#endif // FLASH_H

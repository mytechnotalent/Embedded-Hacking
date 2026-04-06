/**
 * @file flash.c
 * @brief Implementation of on-chip flash read/write driver
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

#include "flash.h"
#include <string.h>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

/**
 * @brief Erase one 4096-byte sector and write data to on-chip flash
 *
 * The target address must be aligned to a 4096-byte sector boundary.
 * The function disables interrupts, erases the containing sector,
 * programs up to len bytes from data, and re-enables interrupts.
 *
 * @param flash_offset Byte offset from the start of flash (must be sector-aligned)
 * @param data         Pointer to the data buffer to write
 * @param len          Number of bytes to write (multiple of FLASH_DRIVER_PAGE_SIZE)
 */
void flash_driver_write(uint32_t flash_offset, const uint8_t *data, uint32_t len) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(flash_offset, FLASH_SECTOR_SIZE);
    flash_range_program(flash_offset, data, len);
    restore_interrupts(ints);
}

/**
 * @brief Read bytes from on-chip flash via the XIP memory map
 *
 * Flash is memory-mapped starting at XIP_BASE (0x10000000). This function
 * copies len bytes beginning at flash_offset into out using the XIP read
 * path, which is always available without erasing.
 *
 * @param flash_offset Byte offset from the start of flash
 * @param out          Pointer to the destination buffer (must be len bytes)
 * @param len          Number of bytes to read
 */
void flash_driver_read(uint32_t flash_offset, uint8_t *out, uint32_t len) {
    const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + flash_offset);
    memcpy(out, flash_target_contents, len);
}

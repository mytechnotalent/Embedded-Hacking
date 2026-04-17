/**
 * @file rp2350_flash.h
 * @brief On-chip flash driver header for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Bare-metal flash erase, program, and read driver using ROM
 * bootrom functions. Write operations run from RAM to avoid
 * XIP conflicts.
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
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
#ifndef __RP2350_FLASH_H
#define __RP2350_FLASH_H

#include "rp2350.h"

/**
  * @brief  Erase the containing sector(s) and program data to flash.
  *
  *         The data buffer must reside in RAM (not flash). Interrupts
  *         are disabled for the duration of the erase/program cycle.
  *         The write length must be a multiple of FLASH_PAGE_SIZE
  *         (256 bytes); pad with 0xFF if necessary.
  *
  * @param  offset byte offset from the start of flash (sector-aligned)
  * @param  data   pointer to the source buffer in RAM
  * @param  len    number of bytes to write
  * @retval None
  */
void flash_write(uint32_t offset, const uint8_t *data, uint32_t len);

/**
  * @brief  Read bytes from on-chip flash via the XIP memory map.
  * @param  offset byte offset from the start of flash
  * @param  out    pointer to the destination buffer
  * @param  len    number of bytes to read
  * @retval None
  */
void flash_read(uint32_t offset, uint8_t *out, uint32_t len);

#endif /* __RP2350_FLASH_H */

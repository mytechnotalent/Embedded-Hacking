/**
  ******************************************************************************
  * @file    rp2350_flash.h
  * @author  Kevin Thomas
  * @brief   On-chip flash driver header for RP2350.
  *
  *          Bare-metal flash erase, program, and read driver using ROM
  *          bootrom functions. Write operations run from RAM to avoid
  *          XIP conflicts.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
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

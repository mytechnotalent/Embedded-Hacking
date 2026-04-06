/**
  ******************************************************************************
  * @file    rp2350_flash.c
  * @author  Kevin Thomas
  * @brief   On-chip flash driver implementation for RP2350.
  *
  *          Erases and programs flash sectors using ROM bootrom
  *          functions. The erase/program trampoline executes from RAM
  *          (placed in .ram_func) because XIP is disabled while the
  *          flash chip is being modified.
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

#include "rp2350_flash.h"

/**
  * @brief  ROM table lookup function pointer type (RP2350 ARM).
  */
typedef void *(*rom_table_lookup_fn)(uint32_t code, uint32_t mask);

/**
  * @brief  ROM void function pointer (no parameters, no return).
  */
typedef void (*rom_void_fn)(void);

/**
  * @brief  ROM flash range erase function pointer.
  */
typedef void (*rom_flash_erase_fn)(uint32_t addr, uint32_t count,
                                   uint32_t block_size, uint8_t block_cmd);

/**
  * @brief  ROM flash range program function pointer.
  */
typedef void (*rom_flash_program_fn)(uint32_t addr, const uint8_t *data,
                                     uint32_t count);

/**
  * @brief  Collection of ROM flash function pointers.
  */
typedef struct
{
  rom_void_fn connect;             /**< @brief Connect to internal flash */
  rom_void_fn exit_xip;            /**< @brief Exit XIP mode */
  rom_flash_erase_fn erase;        /**< @brief Erase flash range */
  rom_flash_program_fn program;    /**< @brief Program flash range */
  rom_void_fn flush_cache;         /**< @brief Flush XIP cache */
  rom_void_fn enter_xip;           /**< @brief Re-enter XIP mode */
} FlashRomFns;

/**
  * @brief  Look up a ROM function by its two-character code.
  * @param  code ROM_FUNC_* code from rp2350.h
  * @retval void* pointer to the ROM function
  */
static void *_rom_func_lookup(uint32_t code)
{
  rom_table_lookup_fn fn =
      (rom_table_lookup_fn)(uintptr_t)(*(uint16_t *)BOOTROM_TABLE_LOOKUP_OFFSET);
  return fn(code, RT_FLAG_FUNC_ARM_SEC);
}

/**
  * @brief  Populate all ROM flash function pointers.
  * @param  fns pointer to the struct to fill
  * @retval None
  */
static void _lookup_rom_fns(FlashRomFns *fns)
{
  fns->connect     = (rom_void_fn)_rom_func_lookup(ROM_FUNC_CONNECT_INTERNAL_FLASH);
  fns->exit_xip    = (rom_void_fn)_rom_func_lookup(ROM_FUNC_FLASH_EXIT_XIP);
  fns->erase       = (rom_flash_erase_fn)_rom_func_lookup(ROM_FUNC_FLASH_RANGE_ERASE);
  fns->program     = (rom_flash_program_fn)_rom_func_lookup(ROM_FUNC_FLASH_RANGE_PROGRAM);
  fns->flush_cache = (rom_void_fn)_rom_func_lookup(ROM_FUNC_FLASH_FLUSH_CACHE);
  fns->enter_xip   = (rom_void_fn)_rom_func_lookup(ROM_FUNC_FLASH_ENTER_CMD_XIP);
}

/**
  * @brief  RAM-resident trampoline that erases and programs flash.
  *
  *         Must not call any function residing in flash. All ROM
  *         function pointers are passed via the fns struct.
  *
  * @param  fns    pointer to ROM function pointers (in RAM/stack)
  * @param  offset byte offset from start of flash
  * @param  data   pointer to source buffer (must be in RAM)
  * @param  len    number of bytes to program
  * @retval None
  */
__attribute__((section(".ram_func"), noinline))
static void _flash_erase_program_ram(const FlashRomFns *fns, uint32_t offset,
                                     const uint8_t *data, uint32_t len)
{
  fns->connect();
  fns->exit_xip();
  uint32_t erase_addr = offset & ~(FLASH_SECTOR_SIZE - 1U);
  uint32_t erase_end = (offset + len + FLASH_SECTOR_SIZE - 1U) & ~(FLASH_SECTOR_SIZE - 1U);
  fns->erase(erase_addr, erase_end - erase_addr, FLASH_BLOCK_SIZE, FLASH_BLOCK_ERASE_CMD);
  fns->program(offset, data, len);
  fns->flush_cache();
  fns->enter_xip();
}

void flash_write(uint32_t offset, const uint8_t *data, uint32_t len)
{
  FlashRomFns fns;
  _lookup_rom_fns(&fns);
  uint32_t primask;
  __asm volatile ("mrs %0, primask" : "=r" (primask));
  __asm volatile ("cpsid i");
  _flash_erase_program_ram(&fns, offset, data, len);
  __asm volatile ("msr primask, %0" :: "r" (primask));
}

void flash_read(uint32_t offset, uint8_t *out, uint32_t len)
{
  const uint8_t *src = (const uint8_t *)(XIP_BASE + offset);
  for (uint32_t i = 0; i < len; i++)
    out[i] = src[i];
}

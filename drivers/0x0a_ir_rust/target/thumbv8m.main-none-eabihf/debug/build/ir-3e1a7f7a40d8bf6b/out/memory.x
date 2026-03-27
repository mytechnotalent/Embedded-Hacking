/*
* SPDX-License-Identifier: MIT OR Apache-2.0
*
* Copyright (c) 2021-2024 The rp-rs Developers
* Copyright (c) 2021 rp-rs organization
* Copyright (c) 2025 Raspberry Pi Ltd.
*/

MEMORY {
      FLASH : ORIGIN = 0x10000000, LENGTH = 2048K
      RAM : ORIGIN = 0x20000000, LENGTH = 512K
      SRAM4 : ORIGIN = 0x20080000, LENGTH = 4K
      SRAM5 : ORIGIN = 0x20081000, LENGTH = 4K
  }

  SECTIONS {
      .start_block : ALIGN(4)
      {
          __start_block_addr = .;
          KEEP(*(.start_block));
      } > FLASH

  } INSERT AFTER .vector_table;

  _stext = ADDR(.start_block) + SIZEOF(.start_block);

  SECTIONS {
      .bi_entries : ALIGN(4)
      {
          __bi_entries_start = .;
          KEEP(*(.bi_entries));
          . = ALIGN(4);
          __bi_entries_end = .;
      } > FLASH
  } INSERT AFTER .text;

  SECTIONS {
      .end_block : ALIGN(4)
      {
          __end_block_addr = .;
          KEEP(*(.end_block));
      } > FLASH

  } INSERT AFTER .uninit;

  PROVIDE(start_to_end = __end_block_addr - __start_block_addr);
  PROVIDE(end_to_start = __start_block_addr - __end_block_addr);
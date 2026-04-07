/**
  ******************************************************************************
  * @file    image_def.c
  * @author  Kevin Thomas
  * @brief   RP2350 IMAGE_DEF block for boot ROM image recognition.
  *
  *          Must appear within the first 4 KB of flash for the boot ROM
  *          to accept the image.
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

#include <stdint.h>

__attribute__((section(".embedded_block"), used))
const uint8_t picobin_block[] = 
{
  0xD3, 0xDE, 0xFF, 0xFF,
  0x42, 0x01, 0x21, 0x10,
  0xFF, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x79, 0x35, 0x12, 0xAB
};

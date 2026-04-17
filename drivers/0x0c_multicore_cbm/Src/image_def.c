/**
 * @file image_def.c
 * @brief RP2350 IMAGE_DEF block for boot ROM image recognition.
 * @author Kevin Thomas
 * @date 2026
 *
 * Must appear within the first 4 KB of flash for the boot ROM
 * to accept the image.
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
#include <stdint.h>

__attribute__((section(".embedded_block"), used))
const uint8_t picobin_block[] = {
  0xD3, 0xDE, 0xFF, 0xFF,
  0x42, 0x01, 0x21, 0x10,
  0xFF, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x79, 0x35, 0x12, 0xAB
};

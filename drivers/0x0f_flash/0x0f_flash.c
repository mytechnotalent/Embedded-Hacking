/**
 * @file 0x0f_flash.c
 * @brief On-chip flash write/read demo using flash.c/flash.h
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
 *
 * -----------------------------------------------------------------------------
 *
 * Demonstrates on-chip flash read/write using the flash driver
 * (flash.h / flash.c). A string is written to the last sector of flash
 * and then read back to verify. The result is printed over UART.
 *
 * Wiring:
 *   No external wiring required
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "flash.h"

#define FLASH_TARGET_OFFSET (FLASH_DRIVER_SIZE_BYTES - FLASH_DRIVER_SECTOR_SIZE)
#define FLASH_WRITE_LEN     FLASH_DRIVER_PAGE_SIZE

int main(void) {
    stdio_init_all();

    static uint8_t write_buf[FLASH_WRITE_LEN];
    static uint8_t read_buf[FLASH_WRITE_LEN];

    memset(write_buf, 0xFF, sizeof(write_buf));
    const char *msg = "Embedded Hacking flash driver demo";
    memcpy(write_buf, msg, strlen(msg));

    flash_driver_write(FLASH_TARGET_OFFSET, write_buf, FLASH_WRITE_LEN);
    flash_driver_read(FLASH_TARGET_OFFSET, read_buf, FLASH_WRITE_LEN);

    printf("Flash readback: %s\r\n", read_buf);

    while (true) {
        tight_loop_contents();
    }
}

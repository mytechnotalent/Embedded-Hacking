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


/**
 * @brief Fill a write buffer with 0xFF and copy the demo string into it
 *
 * @param buf      Destination buffer
 * @param buf_size Size of the buffer in bytes
 */
static void _prepare_write_buf(uint8_t *buf, size_t buf_size) {
    memset(buf, 0xFF, buf_size);
    const char *msg = "Embedded Hacking flash driver demo";
    memcpy(buf, msg, strlen(msg));
}


/**
 * @brief Write the demo string to flash and print the read-back result
 */
static void _write_and_verify(void) {
    static uint8_t write_buf[FLASH_WRITE_LEN];
    static uint8_t read_buf[FLASH_WRITE_LEN];
    _prepare_write_buf(write_buf, sizeof(write_buf));
    flash_driver_write(FLASH_TARGET_OFFSET, write_buf, FLASH_WRITE_LEN);
    flash_driver_read(FLASH_TARGET_OFFSET, read_buf, FLASH_WRITE_LEN);
    printf("Flash readback: %s\r\n", read_buf);
}


/**
 * @brief Application entry point for the on-chip flash demo
 *
 * Writes a demo string to the last flash sector, reads it back,
 * and prints the result over UART.
 *
 * @return int Does not return
 */
int main(void) {
    stdio_init_all();
    _write_and_verify();
    while (true)
        tight_loop_contents();
}

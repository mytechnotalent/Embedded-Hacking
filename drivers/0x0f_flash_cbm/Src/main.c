/**
 * @file main.c
 * @brief On-chip flash write / read demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Writes "Embedded Hacking flash driver demo" to the last
 * sector of flash, reads it back via XIP, and prints the
 * result over UART.
 *
 * Wiring:
 *  GPIO0 -> UART TX (USB-to-UART adapter RX)
 *  GPIO1 -> UART RX (USB-to-UART adapter TX)
 *  No external components required
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
#include "rp2350_flash.h"
#include "rp2350_uart.h"

/**
  * @brief  Target offset: last sector of the 4 MB flash chip.
  */
/** @brief Flash offset for the target storage sector */
#define FLASH_TARGET_OFFSET  (FLASH_SIZE - FLASH_SECTOR_SIZE)

/**
  * @brief  Fill a page buffer with 0xFF and copy the demo string.
  * @param  buf destination buffer (FLASH_PAGE_SIZE bytes in RAM)
  * @retval None
  */
static void prepare_write_buf(uint8_t *buf)
{
  uint32_t i;
  const char *msg = "Embedded Hacking flash driver demo";
  for (i = 0; i < FLASH_PAGE_SIZE; i++)
    buf[i] = 0xFFU;
  for (i = 0; msg[i] != '\0'; i++)
    buf[i] = (uint8_t)msg[i];
  buf[i] = 0x00U;
}

/**
  * @brief  Write the demo string to flash and print the read-back.
  * @retval None
  */
static void write_and_verify(void)
{
  uint8_t write_buf[FLASH_PAGE_SIZE];
  uint8_t read_buf[FLASH_PAGE_SIZE];
  prepare_write_buf(write_buf);
  flash_write(FLASH_TARGET_OFFSET, write_buf, FLASH_PAGE_SIZE);
  flash_read(FLASH_TARGET_OFFSET, read_buf, FLASH_PAGE_SIZE);
  uart_puts("Flash readback: ");
  uart_puts((const char *)read_buf);
  uart_puts("\r\n");
}

int main(void)
{
  write_and_verify();
  while (1)
    __asm volatile ("wfi");
}

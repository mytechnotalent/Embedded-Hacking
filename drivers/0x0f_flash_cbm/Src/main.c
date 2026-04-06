/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   On-chip flash write / read demonstration.
  *
  *          Writes "Embedded Hacking flash driver demo" to the last
  *          sector of flash, reads it back via XIP, and prints the
  *          result over UART.
  *
  *          Wiring:
  *            GPIO0 -> UART TX (USB-to-UART adapter RX)
  *            GPIO1 -> UART RX (USB-to-UART adapter TX)
  *            No external components required
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
#include "rp2350_uart.h"

/**
  * @brief  Target offset: last sector of the 4 MB flash chip.
  */
#define FLASH_TARGET_OFFSET  (FLASH_SIZE - FLASH_SECTOR_SIZE)

/**
  * @brief  Fill a page buffer with 0xFF and copy the demo string.
  * @param  buf destination buffer (FLASH_PAGE_SIZE bytes in RAM)
  * @retval None
  */
static void _prepare_write_buf(uint8_t *buf)
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
static void _write_and_verify(void)
{
  uint8_t write_buf[FLASH_PAGE_SIZE];
  uint8_t read_buf[FLASH_PAGE_SIZE];
  _prepare_write_buf(write_buf);
  flash_write(FLASH_TARGET_OFFSET, write_buf, FLASH_PAGE_SIZE);
  flash_read(FLASH_TARGET_OFFSET, read_buf, FLASH_PAGE_SIZE);
  uart_puts("Flash readback: ");
  uart_puts((const char *)read_buf);
  uart_puts("\r\n");
}

/**
  * @brief  Application entry point for the on-chip flash demo.
  * @retval int does not return
  */
int main(void)
{
  _write_and_verify();
  while (1)
    __asm volatile ("wfi");
}

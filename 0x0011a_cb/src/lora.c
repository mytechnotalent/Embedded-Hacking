// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    lora.c
// Desc:    Implements UART1 driver for REYAX RYLR998 LoRa transceiver.
// Created: 2026

#include "lora.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

static void drain_lora_rx(void)
{
    while (uart_is_readable(LORA_UART)) {
        char c = (char)uart_getc(LORA_UART);
        if (c >= 32 && c <= 126)
            putchar(c);
    }
}

static void send_at_cmd(const char *cmd)
{
    uart_write_blocking(LORA_UART, (const uint8_t *)cmd, strlen(cmd));
    sleep_ms(250);
    drain_lora_rx();
}

static void configure_lora_rf(void)
{
    sleep_ms(1500);
    send_at_cmd("AT\r\n");
    send_at_cmd("AT+NETWORKID=18\r\n");
    send_at_cmd("AT+BAND=915000000\r\n");
    send_at_cmd("AT+PARAMETER=9,7,1,12\r\n");
    send_at_cmd("AT+ADDRESS=2\r\n");
}

void init_lora(void)
{
    uart_init(LORA_UART, LORA_BAUD);
    uart_set_translate_crlf(LORA_UART, false);
    gpio_set_function(LORA_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(LORA_RX_PIN, GPIO_FUNC_UART);
    configure_lora_rf();
}

static char tx_buf[160];
static int tx_len = 0;
static int tx_idx = 0;

void lora_send(const char *msg)
{
    int len = (int)strlen(msg);
    while ((len > 0) && ((msg[len - 1] == '\r') || (msg[len - 1] == '\n')))
        len--;
    tx_len = snprintf(tx_buf, sizeof(tx_buf), "AT+SEND=0,%d,%.*s\r\n", len, len, msg);
    tx_idx = 0;
}

void lora_tick(void)
{
    while ((tx_idx < tx_len) && uart_is_writable(LORA_UART))
        uart_putc_raw(LORA_UART, (uint8_t)tx_buf[tx_idx++]);
    drain_lora_rx();
}

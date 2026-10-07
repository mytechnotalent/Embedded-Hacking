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

static void drain_lora_rx(void) {
    while (uart_is_readable(LORA_UART)) {
        (void)uart_getc(LORA_UART);
    }
}

static void send_at_cmd(const char *cmd) {
    uart_write_blocking(LORA_UART, (const uint8_t *)cmd, strlen(cmd));
    sleep_ms(200);
    drain_lora_rx();
}

static void configure_lora_rf(void) {
    sleep_ms(1500);
    send_at_cmd("AT\r\n");
    send_at_cmd("AT+NETWORKID=18\r\n");
    send_at_cmd("AT+BAND=915000000\r\n");
    send_at_cmd("AT+PARAMETER=9,7,1,12\r\n");
    send_at_cmd("AT+ADDRESS=2\r\n");
}

void init_lora(void) {
    uart_init(LORA_UART, LORA_BAUD);
    uart_set_translate_crlf(LORA_UART, false);
    gpio_set_function(LORA_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(LORA_RX_PIN, GPIO_FUNC_UART);
    configure_lora_rf();
}

void lora_send(const char *msg) {
    char cmd[128];
    int len = (int)strlen(msg);
    snprintf(cmd, sizeof(cmd), "AT+SEND=0,%d,%s\r\n", len, msg);
    uart_write_blocking(LORA_UART, (const uint8_t *)cmd, strlen(cmd));
}

static bool extract_rcv_data(char *line, char *out, size_t max_len) {
    char *c1 = strchr(line, ',');
    if (!c1) return false;
    char *c2 = strchr(c1 + 1, ',');
    if (!c2) return false;
    char *c3 = strchr(c2 + 1, ',');
    if (c3) *c3 = '\0';
    snprintf(out, max_len, "%s", c2 + 1);
    return true;
}

static bool parse_line(char *line, char *out, size_t max_len) {
    if (strstr(line, "+RCV="))
        return extract_rcv_data(line, out, max_len);
    if (strncmp(line, "+", 1) != 0 && strncmp(line, "OK", 2) != 0) {
        snprintf(out, max_len, "%s", line);
        return true;
    }
    return false;
}

static bool process_rx_byte(char c, char *buf, size_t *idx, char *out, size_t max_len) {
    if (c == '\r' || c == '\n') {
        buf[*idx] = '\0';
        bool ok = (*idx > 0) && parse_line(buf, out, max_len);
        *idx = 0;
        return ok;
    }
    if (*idx < 127) buf[(*idx)++] = c;
    return false;
}

bool lora_poll_packet(char *payload_out, size_t max_len) {
    static char buf[128];
    static size_t idx = 0;
    while (uart_is_readable(LORA_UART)) {
        char c = (char)uart_getc(LORA_UART);
        if (process_rx_byte(c, buf, &idx, payload_out, max_len))
            return true;
    }
    return false;
}

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
// GitHub:  https://github.com/mytechnotalent/encryption-c-rp2350
// File:    cli.c
// Desc:    Implements the CLI UART passphrase input interface for Ouroboros.
// Created: 2026

#include "cli.h"
#include "auth.h"
#include "pico/stdlib.h"
#include <stdio.h>

/**
 * @brief Maximum number of passphrase characters accepted from UART.
 *
 * Limits the input buffer to the hardened engine boundary. A null
 * terminator is written after the last character so the buffer must be
 * declared with at least this many bytes.
 */
#define PASS_BUF_LEN AUTH_PASSPHRASE_MAX_LEN

/**
 * @brief Print the hardened passphrase policy hint.
 *
 * The firmware uses the same interactive policy as the host demo:
 * exactly 12 lowercase words separated by spaces.
 *
 * @param None.
 * @return None.
 */
static void print_policy_hint(void)
{
    printf("Enter exactly 12 lowercase words separated by spaces.\r\n");
}

/**
 * @brief Append one received character to the passphrase buffer.
 *
 * Stores printable characters up to the buffer limit minus one to
 * reserve room for a null terminator. Echoes the character back
 * over UART for interactive typing feedback.
 *
 * @param ch Input character value.
 * @param buf Pointer to mutable passphrase buffer.
 * @param idx Pointer to current buffer length.
 * @return None.
 */
static void append_char(int ch, char *buf, size_t *idx)
{
    if (*idx + 1u >= PASS_BUF_LEN) {
        return;
    }
    buf[*idx] = (char)ch;
    *idx += 1u;
    putchar_raw((char)ch);
}

/**
 * @brief Remove one character from the passphrase buffer.
 *
 * Moves the index back by one and emits the backspace-escape
 * sequence to erase the last echoed character on the terminal.
 *
 * @param idx Pointer to current buffer length.
 * @return None.
 */
static void handle_backspace(size_t *idx)
{
    if (*idx == 0u) {
        return;
    }
    *idx -= 1u;
    printf("\b \b");
}

/**
 * @brief Finalise and authenticate the current passphrase buffer.
 *
 * Null-terminates the input, runs the full Ouroboros authentication
 * pipeline via auth_execute, prints policy guidance or authentication
 * failure text as needed, and resets the buffer index for the next
 * prompt cycle.
 *
 * @param buf Pointer to mutable passphrase buffer.
 * @param idx Pointer to current buffer length.
 * @return None.
 */
static void finish_passphrase(char *buf, size_t *idx)
{
    auth_result_t result;
    putchar_raw('\r');
    putchar_raw('\n');
    buf[*idx] = '\0';
    result = auth_execute((const uint8_t *)buf, *idx);
    if (result == AUTH_RESULT_POLICY_VIOLATION) {
        gpio_put(AUTH_LED_PIN, 0);
        print_policy_hint();
    } else if (result != AUTH_RESULT_SUCCESS) {
        gpio_put(AUTH_LED_PIN, 0);
        printf("Authentication failed.\r\n");
    }
    *idx = 0u;
    print_prompt();
}

void print_prompt(void)
{
    printf("\r\n> ");
}

void service_uart(char *buf, size_t *idx)
{
    int ch = getchar_timeout_us(0);
    if (ch == PICO_ERROR_TIMEOUT) {
        tight_loop_contents();
        return;
    }
    if ((ch == '\b') || (ch == 127)) {
        handle_backspace(idx);
        return;
    }
    if ((ch == '\r') || (ch == '\n')) {
        finish_passphrase(buf, idx);
        return;
    }
    append_char(ch, buf, idx);
}

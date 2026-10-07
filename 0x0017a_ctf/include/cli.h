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
// File:    cli.h
// Desc:    Declares the CLI UART passphrase input interface for Ouroboros.
// Created: 2026

#ifndef CLI_H
#define CLI_H

#include <stddef.h>

/**
 * @brief Print the UART passphrase prompt.
 *
 * Emits a minimal shell-style prompt followed by a space so the
 * terminal clearly indicates that hardened passphrase input is expected.
 *
 * @param None.
 * @return None.
 */
void print_prompt(void);

/**
 * @brief Service one UART polling step for passphrase input.
 *
 * Polls stdio for a character, dispatches backspace or newline
 * handling, and appends printable characters to the passphrase
 * buffer. Call repeatedly from the main loop.
 *
 * @param buf Pointer to mutable passphrase buffer.
 * @param idx Pointer to current buffer length.
 * @return None.
 */
void service_uart(char *buf, size_t *idx);

#endif // CLI_H

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
// File:    console.h
// Desc:    Declares the UART console banner and status reporting interface
//          for Operation Black Start.
// Created: 2026

#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * @brief Print the response controller's boot identity and unconditional signal line.
 *
 * Emits the boot banner, operation window, serial configuration banner,
 * unconditional signal quality line, and the interactive command prompt.
 *
 * @param None.
 * @return None.
 */
void print_boot_banner(void);

/**
 * @brief Print the recurring grid classification and dispatch authorization report.
 *
 * Evaluates the current operator state and dispatch state flags and transmits
 * the formatted status report over UART0 with the quarantined frame notification.
 *
 * @param None.
 * @return None.
 */
void print_status(void);

#endif // CONSOLE_H

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
// File:    console.c
// Desc:    Implements the UART console banner and status reporting logic for
//          Operation Black Start.
// Created: 2026

#include "console.h"
#include "grid.h"
#include <stdio.h>

void print_boot_banner(void)
{
    printf("GLOBAL EMBEDDED RESPONSE NETWORK\r\n");
    printf("BLACK START WINDOW: 27 MINUTES\r\n");
    printf("UART0 115200 8N1 | AUTHORIZED LAB CONSOLE\r\n");
    printf("SIGNAL: NORMAL\r\n");
    printf("RESPONSE> ");
}

void print_status(void)
{
    printf("GRID STATUS: %s\r\n", operator_state ? "STABLE" : "CRITICAL");
    printf("DISPATCH PATH: %s\r\n", dispatch_state ? "AUTHORIZED" : "HELD");
    printf("LAST FRAME: QUARANTINED\r\n");
    printf("RESPONSE> ");
}

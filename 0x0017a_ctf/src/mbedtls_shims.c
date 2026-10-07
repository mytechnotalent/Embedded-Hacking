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
// File:    mbedtls_shims.c
// Desc:    Implements platform zeroization shims required by mbedTLS on RP2350.
// Created: 2026

#include "mbedtls/platform_util.h"

/**
 * @brief Securely clear a memory region.
 *
 * Provides the mbedTLS platform zeroization hook for this firmware build.
 * The volatile pointer prevents the compiler from optimizing away the
 * clearing loop.
 *
 * @param buf Pointer to mutable memory region to clear.
 * @param len Number of bytes to clear.
 * @return None.
 */
void mbedtls_platform_zeroize(void *buf, size_t len)
{
    volatile unsigned char *ptr = (volatile unsigned char *)buf;
    while (len-- > 0u) {
        *ptr++ = 0u;
    }
}

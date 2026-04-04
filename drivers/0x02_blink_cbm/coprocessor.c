/**
 * @file coprocessor.c
 * @brief Coprocessor access control enable functions
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
 */

#include "coprocessor.h"
#include "constants.h"

void Enable_Coprocessor(void) {
    uint32_t value;

    // read CPACR value
    value = *CPACR;
    // set CP1: Ctrl access priv coproc 1 bit
    value |= (1U << CPACR_CP1_SHIFT);
    // set CP0: Ctrl access priv coproc 0 bit
    value |= (1U << CPACR_CP0_SHIFT);
    // store value into CPACR
    *CPACR = value;
    // data sync barrier
    __asm__ volatile ("dsb");
    // instruction sync barrier
    __asm__ volatile ("isb");
}

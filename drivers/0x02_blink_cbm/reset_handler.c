/**
 * @file reset_handler.c
 * @brief Reset handler entry point after power-on or system reset
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

#include "reset_handler.h"
#include "stack.h"
#include "xosc.h"
#include "reset.h"
#include "uart.h"
#include "coprocessor.h"

extern int main(void);

void __attribute__((naked, noreturn)) Reset_Handler(void) {
    __asm__ volatile (
        "bl Init_Stack\n\t"
        "bl Init_XOSC\n\t"
        "bl Enable_XOSC_Peri_Clock\n\t"
        "bl Init_Subsystem\n\t"
        "bl UART_Release_Reset\n\t"
        "bl UART_Init\n\t"
        "bl Enable_Coprocessor\n\t"
        "b main\n\t"
    );
}

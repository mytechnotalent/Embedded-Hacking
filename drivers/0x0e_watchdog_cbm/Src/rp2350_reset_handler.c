/**
 * @file rp2350_reset_handler.c
 * @brief Reset handler implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Entry point after power-on or system reset. Initializes the
 * stack, XOSC, subsystem resets, UART, watchdog tick
 * generator, then branches to main().
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
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
#include "rp2350_reset_handler.h"
#include "rp2350_stack.h"
#include "rp2350_xosc.h"
#include "rp2350_reset.h"
#include "rp2350_uart.h"
#include "rp2350_watchdog.h"

extern int main(void);

void __attribute__((naked, noreturn)) Reset_Handler(void)
{
  __asm__ volatile (
    "bl stack_init\n\t"
    "bl xosc_init\n\t"
    "bl xosc_enable_peri_clk\n\t"
    "bl reset_init_subsystem\n\t"
    "bl uart_release_reset\n\t"
    "bl uart_init\n\t"
    "bl watchdog_tick_init\n\t"
    "b main\n\t"
  );
}

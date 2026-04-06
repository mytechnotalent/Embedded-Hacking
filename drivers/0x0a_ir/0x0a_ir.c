/**
 * @file 0x0a_ir.c
 * @brief NEC IR (infrared) receiver driver for the Raspberry Pi Pico 2
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
 *
 * -----------------------------------------------------------------------------
 *
 * This driver demonstrates NEC infrared remote decoding using the
 * ir.c/ir.h driver. The IR receiver module output is connected to GPIO 5.
 * ir_getkey() blocks until a valid NEC frame is received or a timeout
 * occurs. Each decoded command byte is printed over UART in hex and decimal.
 *
 * Wiring:
 *   GPIO5 -> IR receiver module OUT pin  (e.g. VS1838B or TSOP4838)
 *   3.3V  -> IR receiver module VCC
 *   GND   -> IR receiver module GND
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "ir.h"

/** @brief GPIO pin connected to the IR receiver output */
#define IR_GPIO 5

/**
 * @brief Poll for an NEC frame and print the command if received
 */
static void _poll_and_print(void) {
    int command = ir_getkey();
    if (command >= 0)
        printf("NEC command: 0x%02X  (%d)\r\n", command, command);
}

/**
 * @brief Application entry point for the NEC IR receiver demo
 *
 * Initializes the IR receiver on GPIO5 and continuously decodes
 * NEC frames, printing each command byte over UART.
 *
 * @return int Does not return
 */
int main(void) {
    stdio_init_all();
    ir_init(IR_GPIO);
    printf("NEC IR driver initialized on GPIO %d\r\n", IR_GPIO);
    printf("Press a button on your NEC remote...\r\n");
    while (true)
        _poll_and_print();
}

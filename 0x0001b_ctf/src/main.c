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
// File:    main.c
// Desc:    Main entry point for the Operation Black Start CTF relay firmware.
// Created: 2026

#include "console.h"
#include "grid.h"
#include "pico/stdlib.h"

/**
 * @brief Run the recurring UART evidence stream for the isolated CTF image.
 *
 * Initializes the standard I/O subsystems, anchors the quarantined dispatch
 * frame in flash memory, evaluates the frozen telemetry against the compiled
 * safety threshold, prints the startup banner, and repeatedly emits the
 * system status report every second.
 *
 * @param None.
 * @return int Standard exit code (never reached in normal firmware execution).
 */
int main(void)
{
    stdio_init_all();
    retain_dispatch_frame();
    evaluate_grid();
    print_boot_banner();
    while (true) {
        print_status();
        sleep_ms(1000);
    }
    return 0;
}

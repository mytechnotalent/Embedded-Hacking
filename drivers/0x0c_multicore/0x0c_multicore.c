/**
 * @file 0x0c_multicore.c
 * @brief Multicore FIFO messaging demo using multicore.c/multicore.h
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
 * Demonstrates dual-core operation using the multicore driver
 * (multicore.h / multicore.c). Core 0 sends an incrementing counter to
 * core 1 via the FIFO; core 1 returns the value plus one. Both values
 * are printed over UART every second.
 *
 * Wiring:
 *   No external wiring required (dual-core communication is on-chip)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "multicore.h"

static void core1_main(void) {
    while (true) {
        uint32_t value = multicore_driver_pop();
        multicore_driver_push(value + 1u);
    }
}

int main(void) {
    stdio_init_all();
    multicore_driver_launch(core1_main);

    uint32_t counter = 0;
    while (true) {
        multicore_driver_push(counter);
        uint32_t response = multicore_driver_pop();
        printf("core0 sent: %lu, core1 returned: %lu\r\n",
               (unsigned long)counter, (unsigned long)response);
        counter++;
        sleep_ms(1000);
    }
}

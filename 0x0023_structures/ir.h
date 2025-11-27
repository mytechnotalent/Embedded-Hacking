/**
 * @file ir.h
 * @brief Header for NEC IR receiver (decoder) API
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

#ifndef IR_H
#define IR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the IR receiver GPIO
 *
 * Configures the given `pin` as an input with pull-up for the NEC IR receiver.
 * Call this once during board initialization before calling `ir_getkey()`.
 *
 * @param pin GPIO pin number connected to IR receiver output
 */
void ir_init(uint8_t pin);

/**
 * @brief Blocking NEC IR decoder
 *
 * Blocks while waiting for a complete NEC frame. On success returns the
 * decoded command byte (0..255). On timeout or protocol error returns -1.
 *
 * @return decoded command byte (0..255) or -1 on failure
 */
int ir_getkey(void);

#ifdef __cplusplus
}
#endif

#endif // IR_H

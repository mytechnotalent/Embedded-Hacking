/**
 * @file rp2350_ir.h
 * @brief NEC IR receiver driver for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Decodes NEC infrared remote frames on GPIO5 using SIO
 * input reads and TIMER0 TIMERAWL for microsecond timing.
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
#ifndef __RP2350_IR_H
#define __RP2350_IR_H

#include "rp2350.h"

/**
  * @brief  Release TIMER0 from reset in the reset controller.
  * @retval None
  */
void ir_timer_release_reset(void);

/**
  * @brief  Start the TIMER0 tick generator for 1 us ticks at 12 MHz.
  * @retval None
  */
void ir_timer_start_tick(void);

/**
  * @brief  Configure GPIO5 pad and funcsel for SIO input with pull-up.
  * @retval None
  */
void ir_init(void);

/**
  * @brief  Block until a valid NEC frame is received or timeout.
  * @retval int command byte (0-255) on success, -1 on failure
  */
int ir_getkey(void);

#endif /* __RP2350_IR_H */

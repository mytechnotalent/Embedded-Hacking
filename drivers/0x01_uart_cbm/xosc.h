/**
 * @file xosc.h
 * @brief External crystal oscillator (XOSC) initialization and clock enable
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

#ifndef XOSC_H
#define XOSC_H

/**
 * @brief Initialize the external crystal oscillator and wait until stable
 *
 * Configures the XOSC startup delay and frequency range, then polls
 * the status register until the STABLE bit is set.
 *
 * @param   None
 * @retval  None
 */
void Init_XOSC(void);

/**
 * @brief Enable the XOSC peripheral clock
 *
 * Sets the peripheral clock to use XOSC as its auxiliary source and
 * enables it via CLK_PERI_CTRL.
 *
 * @param   None
 * @retval  None
 */
void Enable_XOSC_Peri_Clock(void);

#endif // XOSC_H

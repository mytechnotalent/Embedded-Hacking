/**
 * @file xosc.c
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

#include "xosc.h"
#include "constants.h"

void Init_XOSC(void) {
    // set delay 50,000 cycles
    XOSC->STARTUP = 0x00C4U;
    // set 1_15MHz, freq range, actual 14.5MHz
    XOSC->CTRL = 0x00FABAA0U;
    // wait until stable bit is set
    while ((XOSC->STATUS & (1U << XOSC_STATUS_STABLE_SHIFT)) == 0) {
    }
}

void Enable_XOSC_Peri_Clock(void) {
    uint32_t value;

    // read CLK_PERI_CTRL value
    value = CLOCKS->CLK_PERI_CTRL;
    // set ENABLE bit
    value |= (1U << CLOCKS_CLK_PERI_CTRL_ENABLE_SHIFT);
    // set AUXSRC: XOSC_CLKSRC
    value |= (CLOCKS_CLK_PERI_CTRL_AUXSRC_XOSC << CLOCKS_CLK_PERI_CTRL_AUXSRC_SHIFT);
    // store value into CLK_PERI_CTRL
    CLOCKS->CLK_PERI_CTRL = value;
}

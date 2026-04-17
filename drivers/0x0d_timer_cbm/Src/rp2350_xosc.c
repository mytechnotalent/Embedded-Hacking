/**
 * @file rp2350_xosc.c
 * @brief XOSC driver implementation for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Configures the external crystal oscillator and enables the
 * peripheral clock sourced from XOSC.
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
#include "rp2350_xosc.h"

void xosc_init(void)
{
  XOSC->STARTUP = 0x00C4U;
  XOSC->CTRL = 0x00FABAA0U;
  while ((XOSC->STATUS & (1U << XOSC_STATUS_STABLE_SHIFT)) == 0) {}
}

void xosc_enable_peri_clk(void)
{
  uint32_t value;
  value = CLOCKS->CLK_PERI_CTRL;
  value &= ~CLK_PERI_CTRL_AUXSRC_MASK;
  value |= (1U << CLK_PERI_CTRL_ENABLE_SHIFT);
  value |= (CLK_PERI_CTRL_AUXSRC_XOSC << CLK_PERI_CTRL_AUXSRC_SHIFT);
  CLOCKS->CLK_PERI_CTRL = value;
}

void xosc_set_clk_ref(void)
{
  CLOCKS->CLK_REF_CTRL = CLK_REF_CTRL_SRC_XOSC;
}

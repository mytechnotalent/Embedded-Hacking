/**
 * @file vector_table.c
 * @brief Vector table with initial stack pointer, reset handler, and
 * @author Kevin Thomas
 * @date 2026
 *
 * TIMER0 alarm 0 IRQ handler.
 *
 * Placed in the .vectors section at the start of flash.
 * The Thumb bit (bit 0 = 1) is automatically set by the
 * linker for function pointers in Thumb mode.
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
#include <stdint.h>

extern uint32_t _stack_top;
extern void Reset_Handler(void);
extern void TIMER0_IRQ_0_Handler(void);

/**
  * @brief  Default handler for unused exceptions (infinite loop).
  * @retval None
  */
static void default_handler(void)
{
  while (1) {}
}

typedef void (*vector_func_t)(void);

__attribute__((section(".vectors"), used))
const void *_vectors[17] = {
  &_stack_top,                 // 0:  Initial stack pointer
  Reset_Handler,               // 1:  Reset
  default_handler,            // 2:  NMI
  default_handler,            // 3:  HardFault
  default_handler,            // 4:  MemManage
  default_handler,            // 5:  BusFault
  default_handler,            // 6:  UsageFault
  default_handler,            // 7:  SecureFault
  0,                           // 8:  Reserved
  0,                           // 9:  Reserved
  0,                           // 10: Reserved
  default_handler,            // 11: SVCall
  default_handler,            // 12: DebugMon
  0,                           // 13: Reserved
  default_handler,            // 14: PendSV
  default_handler,            // 15: SysTick
  TIMER0_IRQ_0_Handler,        // 16: IRQ 0 — TIMER0_IRQ_0
};

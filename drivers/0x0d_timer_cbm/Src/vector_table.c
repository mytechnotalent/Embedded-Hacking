/**
  ******************************************************************************
  * @file    vector_table.c
  * @author  Kevin Thomas
  * @brief   Vector table with initial stack pointer, reset handler, and
  *          TIMER0 alarm 0 IRQ handler.
  *
  *          Placed in the .vectors section at the start of flash.
  *          The Thumb bit (bit 0 = 1) is automatically set by the
  *          linker for function pointers in Thumb mode.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include <stdint.h>

extern uint32_t _stack_top;
extern void Reset_Handler(void);
extern void TIMER0_IRQ_0_Handler(void);

/**
  * @brief  Default handler for unused exceptions (infinite loop).
  * @retval None
  */
static void _default_handler(void)
{
  while (1) {
  }
}

typedef void (*vector_func_t)(void);

/**
  * @brief Vector table placed in .vectors section
  *
  *        Entry 0:     Initial SP
  *        Entry 1:     Reset handler
  *        Entries 2-15: System exceptions (NMI, HardFault, etc.)
  *        Entry 16:    IRQ 0 = TIMER0_IRQ_0
  */
__attribute__((section(".vectors"), used))
const void *_vectors[17] = {
  &_stack_top,                 // 0:  Initial stack pointer
  Reset_Handler,               // 1:  Reset
  _default_handler,            // 2:  NMI
  _default_handler,            // 3:  HardFault
  _default_handler,            // 4:  MemManage
  _default_handler,            // 5:  BusFault
  _default_handler,            // 6:  UsageFault
  _default_handler,            // 7:  SecureFault
  0,                           // 8:  Reserved
  0,                           // 9:  Reserved
  0,                           // 10: Reserved
  _default_handler,            // 11: SVCall
  _default_handler,            // 12: DebugMon
  0,                           // 13: Reserved
  _default_handler,            // 14: PendSV
  _default_handler,            // 15: SysTick
  TIMER0_IRQ_0_Handler,        // 16: IRQ 0 — TIMER0_IRQ_0
};

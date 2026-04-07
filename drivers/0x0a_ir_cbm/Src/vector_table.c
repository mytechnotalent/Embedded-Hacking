/**
  ******************************************************************************
  * @file    vector_table.c
  * @author  Kevin Thomas
  * @brief   Vector table with initial stack pointer and reset handler.
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

typedef void (*vector_func_t)(void);

__attribute__((section(".vectors"), used))
const void *_vectors[2] = 
{
  &_stack_top,
  Reset_Handler
};

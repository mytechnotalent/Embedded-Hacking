/**
  ******************************************************************************
  * @file    rp2350_ir.h
  * @author  Kevin Thomas
  * @brief   NEC IR receiver driver for RP2350.
  *
  *          Decodes NEC infrared remote frames on GPIO5 using SIO
  *          input reads and TIMER0 TIMERAWL for microsecond timing.
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

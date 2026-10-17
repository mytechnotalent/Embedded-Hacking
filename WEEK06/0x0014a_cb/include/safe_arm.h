/**
 * @file safe_arm.h
 * @brief Electronic Safe-and-Arm (ESA) controller interface
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef SAFE_ARM_H
#define SAFE_ARM_H

#include <stdbool.h>
#include <stdint.h>

#define STATE_WARHEAD_ARMED 0x01
#define STATE_WARHEAD_SAFE  0x03
#define STATE_TAMPER_TRIP   0xFF

/**
 * @brief Initialize the safe-and-arm controller subsystem.
 * @param void No parameters.
 * @return void No return value.
 */
void init_safe_arm_controller(void);

/**
 * @brief Process one iteration of the safe-and-arm event loop.
 * @param void No parameters.
 * @return void No return value.
 */
void process_safe_arm_tick(void);

#endif /* SAFE_ARM_H */

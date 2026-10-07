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

void init_safe_arm_controller(void);
void process_safe_arm_tick(void);

#endif /* SAFE_ARM_H */

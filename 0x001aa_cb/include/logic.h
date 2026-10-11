/**
 * @file logic.h
 * @brief Operation Iron Net Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#define STATE_ENV_ARMED       0x01
#define STATE_ENV_AUTHORIZED  0x03
#define STATE_ENV_LOCKOUT     0xFF

/**
 * @brief Initialize the strike logic, token, radio, and feedback.
 * @param void No parameters.
 * @return void No return value.
 */
void init_strike_controller(void);

/**
 * @brief Evaluate the environment and fire the payload when authorized.
 * @param void No parameters.
 * @return void No return value.
 */
void evaluate_environment(void);

#endif // LOGIC_H

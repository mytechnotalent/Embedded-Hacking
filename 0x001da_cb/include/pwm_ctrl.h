/**
 * @file pwm_ctrl.h
 * @brief Operation Broken Wing Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef PWM_CTRL_H
#define PWM_CTRL_H
/**
 * @brief Initialize the servo PWM output.
 *
 * @param void No parameters.
 * @return void
 */
void init_servo(void);
/**
 * @brief Deploy the payload via servo actuation.
 *
 * @param void No parameters.
 * @return void
 */
void deploy_payload(void);
/**
 * @brief Lock the payload at the given altitude.
 *
 * @param altitude Altitude at which to lock the payload.
 * @return void
 */
void lock_payload(int altitude);
#endif

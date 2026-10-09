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
void init_servo(void);
void deploy_payload(void);
void lock_payload(int altitude);
#endif

/**
 * @file pwm_ctrl.c
 * @brief Operation Broken Wing Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "pwm_ctrl.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>

#define SERVO_PIN 16
static int pwm_slice = 0;

void init_servo(void) {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_wrap(pwm_slice, 39062);
    pwm_set_clkdiv(pwm_slice, 64.0f);
    pwm_set_enabled(pwm_slice, true);
    pwm_set_gpio_level(SERVO_PIN, 0);
}

void deploy_payload(void) {
    printf("[*] PWM ACTIVE: PAYLOAD BAY DEPLOYED.\r\n");
    pwm_set_gpio_level(SERVO_PIN, 4000);
}

void lock_payload(int altitude) {
    printf("[-] ALTITUDE %d. PWM LOCKED: SAFETY ENGAGED.\r\n", altitude);
    pwm_set_gpio_level(SERVO_PIN, 0);
}

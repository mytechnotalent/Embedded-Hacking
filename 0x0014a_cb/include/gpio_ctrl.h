/**
 * @file gpio_ctrl.h
 * @brief GPIO hardware control interface for Operation Zero Hour ESA
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef GPIO_CTRL_H
#define GPIO_CTRL_H

#include <stdbool.h>

#define LED_RED_PIN       16
#define LED_GREEN_PIN     17
#define TAMPER_WIRE_PIN   15

void init_gpio(void);
void set_led_armed(void);
void set_led_safe(void);
void set_led_tamper_flash(bool state);
bool is_tamper_wire_intact(void);

#endif /* GPIO_CTRL_H */

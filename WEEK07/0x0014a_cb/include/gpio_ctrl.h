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

/**
 * @brief Initialize LED and tamper wire GPIO pins.
 * @param void No parameters.
 * @return void No return value.
 */
void init_gpio(void);

/**
 * @brief Drive the armed indication LEDs (red on, green off).
 * @param void No parameters.
 * @return void No return value.
 */
void set_led_armed(void);

/**
 * @brief Drive the safe indication LEDs (green on, red off).
 * @param void No parameters.
 * @return void No return value.
 */
void set_led_safe(void);

/**
 * @brief Drive the tamper flash indication on the red LED.
 * @param state True to turn the red LED on, false to turn it off.
 * @return void No return value.
 */
void set_led_tamper_flash(bool state);

/**
 * @brief Read the tamper wire loop state.
 * @param void No parameters.
 * @return true when the tamper wire is intact, false when cut.
 */
bool is_tamper_wire_intact(void);

#endif /* GPIO_CTRL_H */

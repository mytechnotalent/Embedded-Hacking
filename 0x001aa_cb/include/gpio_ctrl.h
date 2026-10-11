/**
 * @file gpio_ctrl.h
 * @brief Operation Iron Net Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef GPIO_CTRL_H
#define GPIO_CTRL_H

#include <stdbool.h>

#define PAYLOAD_PIN     18
#define LED_RED_PIN     16
#define LED_GREEN_PIN   17
#define TAMPER_PIN      15

/**
 * @brief Initialize the status LEDs, payload, and tamper pins.
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
 * @brief Drive the authorized indication LEDs (green on, red off).
 * @param void No parameters.
 * @return void No return value.
 */
void set_led_authorized(void);

/**
 * @brief Drive the tamper flash indication on the red LED.
 * @param state True to turn the red LED on, false to turn it off.
 * @return void No return value.
 */
void set_led_tamper_flash(bool state);

/**
 * @brief Read the enclosure tamper loop state.
 * @param void No parameters.
 * @return true when the tamper wire is intact, false when cut.
 */
bool is_tamper_wire_intact(void);

/**
 * @brief Pulse the payload marker output to deploy the marker.
 * @param void No parameters.
 * @return void No return value.
 */
void payload_deploy(void);

#endif // GPIO_CTRL_H

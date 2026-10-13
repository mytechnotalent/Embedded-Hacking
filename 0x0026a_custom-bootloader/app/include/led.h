/**
 * @file led.h
 * @brief LED controller driven by decoded NEC IR commands.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED controller state for the three IR-controlled LEDs.
 *
 * Maps each logical LED (1-3) to its GPIO pin and remembers whether the
 * LED is currently on. Build a default instance with led_default() and
 * drive it with led_init() and led_handle_ir().
 */
typedef struct {
    uint8_t led1_pin;
    uint8_t led2_pin;
    uint8_t led3_pin;
    bool led1_state;
    bool led2_state;
    bool led3_state;
} simple_led_ctrl_t;

/**
 * @brief Build the default three-LED configuration (pins 16-18, all off).
 * @param void No parameters.
 * @return simple_led_ctrl_t Default LED controller state.
 */
simple_led_ctrl_t led_default(void);

/**
 * @brief Configure the three LED GPIO pins as outputs.
 * @param leds Pointer to the LED controller to initialize.
 * @return void No return value.
 */
void led_init(simple_led_ctrl_t *leds);

/**
 * @brief Dispatch a decoded NEC command to the matching LED.
 * @param leds Pointer to the LED controller to drive.
 * @param key  Decoded NEC command byte, or a negative value to ignore.
 * @return void No return value.
 */
void led_handle_ir(simple_led_ctrl_t *leds, int key);

#ifdef __cplusplus
}
#endif

#endif // LED_H
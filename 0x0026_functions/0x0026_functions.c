#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "ir.h"

#define IR_PIN 5

typedef struct {
    uint8_t led1_pin;
    uint8_t led2_pin;
    uint8_t led3_pin;
    bool led1_state;
    bool led2_state;
    bool led3_state;
} simple_led_ctrl_t;

/**
 * @brief Map NEC IR command code to LED number
 *
 * Translates a received NEC IR command code to a logical LED number.
 * Supports three button mappings for controlling individual LEDs.
 *
 * @param ir_command NEC command code from IR receiver
 * @return int LED number (1-3) or 0 if no match
 */
int ir_to_led_number(int ir_command) {
    if (ir_command == 0x0C) return 1;
    if (ir_command == 0x18) return 2;
    if (ir_command == 0x5E) return 3;
    return 0;
}

/**
 * @brief Get GPIO pin number for a given LED number
 *
 * Retrieves the GPIO pin associated with a logical LED number
 * from the LED controller structure.
 *
 * @param leds Pointer to LED controller structure
 * @param led_num LED number (1-3)
 * @return uint8_t GPIO pin number or 0 if invalid
 */
uint8_t get_led_pin(simple_led_ctrl_t *leds, int led_num) {
    if (led_num == 1) return leds->led1_pin;
    if (led_num == 2) return leds->led2_pin;
    if (led_num == 3) return leds->led3_pin;
    return 0;
}

/**
 * @brief Turn off all LEDs in the controller
 *
 * Sets all three LED GPIO outputs to low, turning off all LEDs.
 *
 * @param leds Pointer to LED controller structure
 */
void leds_all_off(simple_led_ctrl_t *leds) {
    gpio_put(leds->led1_pin, false);
    gpio_put(leds->led2_pin, false);
    gpio_put(leds->led3_pin, false);
}

/**
 * @brief Blink an LED pin a specified number of times
 *
 * Toggles the specified GPIO pin on and off for the given count,
 * with configurable delay between transitions.
 *
 * @param pin GPIO pin number to blink
 * @param count Number of blink cycles
 * @param delay_ms Delay in milliseconds for on/off periods
 */
void blink_led(uint8_t pin, uint8_t count, uint32_t delay_ms) {
    for (uint8_t i = 0; i < count; i++) {
        gpio_put(pin, true);
        sleep_ms(delay_ms);
        gpio_put(pin, false);
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Process IR command and activate corresponding LED
 *
 * Handles incoming IR commands by turning off all LEDs, mapping the
 * command to an LED, blinking it, then holding it steady.
 *
 * @param ir_command NEC command code from IR receiver
 * @param leds Pointer to LED controller structure
 * @param blink_count Number of blinks before steady state
 * @return int LED number activated (1-3), 0 if none, -1 if invalid
 */
int process_ir_led_command(int ir_command, simple_led_ctrl_t *leds, uint8_t blink_count) {
    if (!leds || ir_command < 0) return -1;
    leds_all_off(leds);
    int led_num = ir_to_led_number(ir_command);
    if (led_num == 0) return 0;
    uint8_t pin = get_led_pin(leds, led_num);
    blink_led(pin, blink_count, 50);
    gpio_put(pin, true);
    return led_num;
}

int main(void) {
    stdio_init_all();

    simple_led_ctrl_t leds = {
        .led1_pin = 16,
        .led2_pin = 17,
        .led3_pin = 18,
        .led1_state = false,
        .led2_state = false,
        .led3_state = false
    };

    // Initialize LED GPIOs
    gpio_init(leds.led1_pin); gpio_set_dir(leds.led1_pin, GPIO_OUT);
    gpio_init(leds.led2_pin); gpio_set_dir(leds.led2_pin, GPIO_OUT);
    gpio_init(leds.led3_pin); gpio_set_dir(leds.led3_pin, GPIO_OUT);

    // Initialize IR receiver
    ir_init(IR_PIN);
    printf("IR receiver on GPIO %d ready\n", IR_PIN);
    printf("Press remote buttons to control LEDs with blink effects!\n");

    while (true) {
        int key = ir_getkey();
        if (key >= 0) {
            printf("NEC command: 0x%02X\n", key);

            // Process the IR command with 3 blinks before steady state
            int activated_led = process_ir_led_command(key, &leds, 3);

            if (activated_led > 0) {
                printf("LED %d activated on GPIO %d\n", activated_led,
                       (activated_led == 1) ? leds.led1_pin :
                       (activated_led == 2) ? leds.led2_pin : leds.led3_pin);
            } else if (activated_led == 0) {
                printf("No LED matched or chase effect played\n");
            } else {
                printf("Invalid command or null pointer\n");
            }

            sleep_ms(10);
        } else {
            sleep_ms(1);
        }
    }
}

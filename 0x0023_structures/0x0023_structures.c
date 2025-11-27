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

    gpio_init(leds.led1_pin); gpio_set_dir(leds.led1_pin, GPIO_OUT);
    gpio_init(leds.led2_pin); gpio_set_dir(leds.led2_pin, GPIO_OUT);
    gpio_init(leds.led3_pin); gpio_set_dir(leds.led3_pin, GPIO_OUT);

    ir_init(IR_PIN);
    printf("IR receiver on GPIO %d ready\n", IR_PIN);

    while (true) {
        int key = ir_getkey();
        if (key >= 0) {
            printf("NEC command: 0x%02X\n", key);

            // turn all off first
            leds.led1_state = false;
            leds.led2_state = false;
            leds.led3_state = false;

            // check NEC codes
            if (key == 0x0C) leds.led1_state = true;   // GPIO16
            if (key == 0x18) leds.led2_state = true;   // GPIO17
            if (key == 0x5E) leds.led3_state = true;   // GPIO18

            // apply states
            gpio_put(leds.led1_pin, leds.led1_state);
            gpio_put(leds.led2_pin, leds.led2_state);
            gpio_put(leds.led3_pin, leds.led3_state);

            sleep_ms(10);
        } else {
            sleep_ms(1);
        }
    }
}

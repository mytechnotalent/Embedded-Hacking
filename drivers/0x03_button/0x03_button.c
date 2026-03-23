/**
 * @file 0x03_button.c
 * @brief Button demonstration: debounced press mirrors to LED + UART report
 * @author Kevin Thomas
 * @date 2025
 *
 * MIT License
 *
 * Copyright (c) 2025 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * -----------------------------------------------------------------------------
 *
 * Demonstrates GPIO input using the button driver (button.h / button.c).
 * The onboard LED mirrors the button state and every edge transition is
 * reported over UART.
 *
 * Wiring:
 *   GPIO15 -> One leg of push button
 *   GND    -> Other leg of push button
 *   GPIO25 -> Onboard LED (no external wiring needed)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "button.h"

#define BUTTON_PIN  15
#define LED_PIN     25
#define DEBOUNCE_MS 20


/**
 * @brief Poll button state and report edge transitions over UART
 *
 * Reads the debounced button state, mirrors it to the LED, and prints
 * a message when the state changes from the previous reading.
 *
 * @param last_state Pointer to the stored previous button state
 */
static void _poll_button(bool *last_state) {
    bool pressed = button_is_pressed(BUTTON_PIN);
    button_led_set(LED_PIN, pressed);
    if (pressed != *last_state) {
        printf("Button: %s\r\n", pressed ? "PRESSED" : "RELEASED");
        *last_state = pressed;
    }
}


/**
 * @brief Application entry point for the button debounce demo
 *
 * Initializes button and LED, then continuously polls button state
 * and mirrors it to the LED with UART reporting on edge transitions.
 *
 * @return int Does not return
 */
int main(void) {
    stdio_init_all();
    button_init(BUTTON_PIN, DEBOUNCE_MS);
    button_led_init(LED_PIN);
    printf("Button driver initialized: button=GPIO%d  led=GPIO%d\r\n", BUTTON_PIN, LED_PIN);
    bool last_state = false;
    while (true) {
        _poll_button(&last_state);
        sleep_ms(10);
    }
}

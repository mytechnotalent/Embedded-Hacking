/**
 * @file main.c
 * @brief Button demonstration: debounced press mirrors to LED + UART report.
 * @author Kevin Thomas
 * @date 2026
 *
 * Demonstrates GPIO input using the button driver. The onboard
 * LED mirrors the button state and every edge transition is
 * reported over UART.
 *
 * Wiring:
 *  GPIO15 -> One leg of push button
 *  GND    -> Other leg of push button
 *  GPIO25 -> Onboard LED (no external wiring needed)
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
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
 */
#include "rp2350_button.h"
#include "rp2350_led.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/** @brief Button debounce window in milliseconds */
#define DEBOUNCE_MS  20
/** @brief Button polling interval in milliseconds */
#define POLL_DELAY_MS 10

/**
  * @brief  Drive the LED to match the current button state.
  * @param  pressed true if button is pressed, false if released
  * @retval None
  */
static void set_led_state(bool pressed)
{
  if (pressed)
    led_on(LED_PIN);
  else
    led_off(LED_PIN);
}

/**
  * @brief  Report a button edge transition over UART.
  * @param  pressed    current button state
  * @param  last_state pointer to the stored previous button state
  * @retval None
  */
static void report_edge(bool pressed, bool *last_state)
{
  if (pressed != *last_state) 
  {
    if (pressed)
      uart_puts("Button: PRESSED\r\n");
    else
      uart_puts("Button: RELEASED\r\n");
    *last_state = pressed;
  }
}

/**
  * @brief  Poll the button, mirror to LED, and report edges.
  * @param  last_state pointer to the stored previous button state
  * @retval None
  */
static void poll_button(bool *last_state)
{
  bool pressed;
  pressed = button_is_pressed(BUTTON_PIN);
  set_led_state(pressed);
  report_edge(pressed, last_state);
}

int main(void)
{
  button_init(BUTTON_PIN, DEBOUNCE_MS);
  led_init(LED_PIN);
  uart_puts("Button driver initialized: button=GPIO15  led=GPIO25\r\n");
  bool last_state = false;
  while (1) 
  {
    poll_button(&last_state);
    delay_ms(POLL_DELAY_MS);
  }
}

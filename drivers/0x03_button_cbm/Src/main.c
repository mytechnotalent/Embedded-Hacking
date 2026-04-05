/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   Button demonstration: debounced press mirrors to LED + UART report.
  *
  *          Demonstrates GPIO input using the button driver. The onboard
  *          LED mirrors the button state and every edge transition is
  *          reported over UART.
  *
  *          Wiring:
  *            GPIO15 -> One leg of push button
  *            GND    -> Other leg of push button
  *            GPIO25 -> Onboard LED (no external wiring needed)
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "rp2350_button.h"
#include "rp2350_led.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

#define DEBOUNCE_MS  20
#define POLL_DELAY_MS 10

/**
  * @brief  Drive the LED to match the current button state.
  * @param  pressed true if button is pressed, false if released
  * @retval None
  */
static void _set_led_state(bool pressed)
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
static void _report_edge(bool pressed, bool *last_state)
{
  if (pressed != *last_state) {
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
static void _poll_button(bool *last_state)
{
  bool pressed;
  pressed = button_is_pressed(BUTTON_PIN);
  _set_led_state(pressed);
  _report_edge(pressed, last_state);
}

/**
  * @brief  Application entry point for the button debounce demo.
  * @retval int does not return
  */
int main(void)
{
  button_init(BUTTON_PIN, DEBOUNCE_MS);
  led_init(LED_PIN);
  uart_puts("Button driver initialized: button=GPIO15  led=GPIO25\r\n");
  bool last_state = false;
  while (1) {
    _poll_button(&last_state);
    delay_ms(POLL_DELAY_MS);
  }
}

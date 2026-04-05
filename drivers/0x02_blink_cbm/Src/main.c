/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   Blink demonstration: toggle onboard LED every 500 ms.
  *
  *          Demonstrates GPIO output control using the blink driver.
  *          The onboard LED on GPIO 25 is toggled every 500 ms and the
  *          current state is reported over UART.
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

#include "rp2350_led.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

#define BLINK_DELAY_MS 500

/**
  * @brief  Print the current LED state over UART.
  * @param  pin GPIO pin number to query
  * @retval None
  */
static void _print_led_state(uint32_t pin)
{
  if (led_get_state(pin))
    uart_puts("LED: ON\r\n");
  else
    uart_puts("LED: OFF\r\n");
}

/**
  * @brief  Application entry point for the LED blink demo.
  * @retval int does not return
  */
int main(void)
{
  led_init(LED_PIN);
  uart_puts("LED driver initialized on GPIO 25\r\n");
  while (1) {
    led_toggle(LED_PIN);
    _print_led_state(LED_PIN);
    delay_ms(BLINK_DELAY_MS);
  }
}
